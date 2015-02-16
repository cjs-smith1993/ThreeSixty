#include "server.h"

ConcurrentQueue q;
std::mutex qMutex;

void canonicalizeURI(std::string root, char URI[]) {
	std::string rootCopy = root.c_str();
	// dprintf("\nBEFORE\nroot: %s\nURI: %s", rootCopy.c_str(), URI);

	if (rootCopy[0] != '/') {
		rootCopy = "/" + rootCopy;
	}
	if (rootCopy[rootCopy.length()-1] == '/') {
		rootCopy = rootCopy.substr(0, rootCopy.length()-1);
	}
	rootCopy = "." + rootCopy;

	if (URI[strlen(URI)-1] == '/') {
		URI[strlen(URI)-1] = '\0';
	}
	// dprintf("\nAFTER\nroot: %s\nURI: %s", rootCopy.c_str(), URI);

	std::string tempURI = rootCopy + std::string(URI);
	strcpy(URI, tempURI.c_str());
}

bool isDirectory(char URI[]) {
	struct stat filestat;
	int statError = stat(URI, &filestat);
	return !statError && S_ISDIR(filestat.st_mode);
}

std::string getMessage(int status) {
	std::string message;

	switch(status) {
		case 200:
			message = "OK";
			break;
		case 404:
			message = "Not Found";
			break;
	}

	return message;
}

int getStatus(int sock, char URI[]) {
	int status = 200;
	struct stat filestat;
	int statError = stat(URI, &filestat);

	if (!statError) {
		if (S_ISREG(filestat.st_mode)) {
			// dprintf("Found a file\n");
		}
		else if (S_ISDIR(filestat.st_mode)) {
			// dprintf("Found a directory\n");
			//check for index.html page
			char indexURI[LINE_LENGTH];
			sprintf(indexURI, "%s/index.html", URI);

			stat(indexURI, &filestat);
			if (S_ISREG(filestat.st_mode)) { //use the index.html page
				sprintf(URI, "%s", indexURI);
				dprintf("%s\n", URI);
			}
			else { //output a directory listing
				// dprintf("%s is not an index file\n", indexURI);
				// isDirectory = true;
			}
		}
	}
	else {
		status = 404;
		sprintf(URI, "404.html");
	}

	return status;
}

void writeStatusLine(int sock, int status, std::string message) {
	char statusLine[LINE_LENGTH];
	sprintf(statusLine, "HTTP/1.0 %d %s\r\n", status, message.c_str());
	write(sock, statusLine, strlen(statusLine));
	dprintf("%s", statusLine);
}

std::string getContentType(int sock, char URI[]) {
	// dprintf("file type of: %s\n", URI);
	const char* fileType = strrchr(URI, '.');
	const char* type;
	// dprintf("%s\n", fileType);
	if (!fileType || strlen(fileType) <= 1 || strchr(fileType, '/')) { //directory
		type = "text/html";
	}
	else if (strcmp(fileType, ".html") == 0) {
		type = "text/html";
	}
	else if (strcmp(fileType, ".txt") == 0) {
		type = "text/plain";
	}
	else if (strcmp(fileType, ".jpg") == 0) {
		type = "image/jpg";
	}
	else if (strcmp(fileType, ".gif") == 0) {
		type = "image/gif";
	}
	else if (strcmp(fileType, ".cgi") == 0 || strcmp(fileType, ".pl") == 0) {
		type = "dynamic";
	}
	else {
		type = "text/plain";
		dprintf("Unknown file type\n");
	}

	return type;
}

void writeContentTypeLine(int sock, std::string type) {
	char contentTypeLine[LINE_LENGTH];
	sprintf(contentTypeLine, "Content-Type: %s\r\n", type.c_str());
	write(sock, contentTypeLine, strlen(contentTypeLine));
	dprintf("%s", contentTypeLine);
}

int getContentLength(int sock, char URI[]) {
	int fileLength = -1;
	struct stat filestat;
	stat(URI, &filestat);
	if (fopen(URI, "r") && S_ISREG(filestat.st_mode)) {
		fileLength = filestat.st_size;
	}
	return fileLength;
}

void writeContentLengthLine(int sock, int fileLength) {
	char contentLength[LINE_LENGTH];
	sprintf(contentLength, "Content-Length: %d\r\n", fileLength);
	write(sock, contentLength, strlen(contentLength));
	dprintf("%s", contentLength);
}

int generateDirectory(char URI[], char buf[]) {
	dprintf("%s\n", URI);
	strcpy(buf, "<html>\r\n<body>\r\n<h1>Index of ");
	strcpy(buf, URI);
	strcpy(buf, "</h1>\r\n<ul>\r\n");

	DIR* d;
	d = opendir(URI);
	struct dirent* dir;
	if (d) {
		while ((dir = readdir(d))) {
			if (strcmp(dir->d_name, ".") == 0) {
				continue;
			}
			strcat(buf, "<li><a href=\"");
			strcat(buf, URI);
			strcat(buf, "/");
			strcat(buf, dir->d_name);
			strcat(buf, "\"> ");
			strcat(buf, dir->d_name);
			strcat(buf, "</a></li>\r\n");
		}

		closedir(d);
	}

	strcat(buf, "</ul>\r\n</body>\r\n</html>");

	return strlen(buf);
}

void writeEndOfHeaders(int sock) {
	char other[] = "Connection: close\r\n";
	write(sock, other, strlen(other));
	dprintf("%s", other);

	//write a blank line
	char blank[] = "\r\n";
	write(sock, blank, strlen(blank));
	dprintf("%s\n", blank);
}

void writeFile(int sock, char URI[], int fileLength) {
	dprintf("about to read from %s\n", URI);
	FILE* file = fopen(URI, "r");
	int fd = fileno(file);
	off_t* offset;
	if(_sendfile(sock, fd, NULL, fileLength) < 0) {
		dprintf("Error writing file to socket: %d", errno);
	}
	fclose(file);
}

void writeFile(int sock, char dirBuf[]) {
	dprintf("about to read directory listing\n%s", dirBuf);
	write(sock, dirBuf, strlen(dirBuf));
}

void runCGI(int sock, char URI[], std::vector<char *> headerLines) {
	dprintf("running CGI script at %s\n", URI);

	// redirect output
	int ServeToCGIpipefd[2];
	int CGIToServepipefd[2];
	pipe(ServeToCGIpipefd);
	pipe(CGIToServepipefd);

	int pid = fork();
	if (pid == 0) {

		close(ServeToCGIpipefd[1]);		// close the write side of the pipe from the server
		dup2(ServeToCGIpipefd[0], 0);	// dup the pipe to stdin
		close(CGIToServepipefd[0]);		// close the read side of the pipe to the server
		dup2(CGIToServepipefd[1], 1);	// dup the pipe to stdout

		int numArgs = headerLines.size();
		char* args[numArgs+1];
		for (int i = 0; i < numArgs; i++) {
			args[i] = headerLines[i];
			// dprintf("%s", args[i]);
		}
		args[numArgs] = NULL;

		char* env[1];
		env[0] = NULL;
		execve(URI, args, env);
	}
	else {
		close(ServeToCGIpipefd[0]);		// close the read side of the pipe to the CGI script
		close(CGIToServepipefd[1]);		// close the write side of the pipe from the CGI script

		// if request is a POST
		// 	get content length from the request headers
		// 	amtread = 0
		// 	while (amtread < contentlength && amt = read(socket, buffer, MAXBUFLEN) )
		// 		amtread += amt
		// 		write (ServeToCGIpipefd[1], buffer, amt);

		// Read from the CGIToServepipefd[0] until you get an error and write this data to the socket
		char buf[BUFFER_LENGTH];
		int totalNumRead = 0;
		int numRead = 0;
		while ((numRead = read(CGIToServepipefd[0], buf+totalNumRead, 1))) {
			totalNumRead += numRead;
		}
		buf[totalNumRead] = '\0';
		dprintf("%s", buf);
		write(sock, buf, strlen(buf));

		close(ServeToCGIpipefd[1]); // all done, close the pipe
		close(CGIToServepipefd[0]); // all done, close the pipe

		shutdown(sock, SHUT_RDWR);
		close(sock);

		dprintf("----------------\n");
	}
}

void serve(int tid, std::string rootPath) {
	while(true) {
		qMutex.lock();
		int sock = q.pop();
		printf("serving socket %d with thread %d\n", sock, tid);

		dprintf("allow socket to be reused\n");
		int optval = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		char cmd[LINE_LENGTH];
		char URI[LINE_LENGTH];
		memset(URI, 0, LINE_LENGTH);
		double version;

		//read the request line
		char* line = GetLine(sock);
		sscanf(line, "%s %s HTTP/%lf", cmd, URI, &version);

		//normalize root path and URI
		canonicalizeURI(rootPath, URI);
		dprintf("Command: %s\nURI: %s\nHTTP version: %lf\n", cmd, URI, version);

		//read the rest of the request headers
		std::vector<char*> headerLines;
		GetHeaderLines(headerLines, sock, true);

		// for (int i = 0; i < headerLines.size(); i++) {
		// 	dprintf("%s\n", headerLines[i]);
		// }

		//write the status line
		int status = getStatus(sock, URI);
		std::string message = getMessage(status);
		bool isDir = isDirectory(URI);
		writeStatusLine(sock, status, message);

		//write the Content-Type line
		std::string type = getContentType(sock, URI);
		if (strcmp(type.c_str(), "dynamic") == 0) { // handle CGI scripts separately
			runCGI(sock, URI, headerLines);
			return;
		}
		writeContentTypeLine(sock, type);

		//write the Content-Length line
		char dirBuf[BUFFER_LENGTH];
		int fileLength = isDir ? generateDirectory(URI, dirBuf) : getContentLength(sock, URI);
		writeContentLengthLine(sock, fileLength);

		//write the rest of the response headers and a blank line
		writeEndOfHeaders(sock);

		//write the file
		isDir ? writeFile(sock, dirBuf) : writeFile(sock, URI, fileLength);

		shutdown(sock, SHUT_RDWR);
		close(sock);

		dprintf("----------------\n");
	}
}

int main(int argc, char* argv[]) {
	//lock signal semaphore
	qMutex.lock();

	//parse arguments
	if (argc < 4) {
		printf("Usage: server <port number> <num threads> <dir>\n");
		exit(0);
	}

	int port = atoi(argv[1]);
	int numThreads = atoi(argv[2]);
	char* dir = argv[3];

	//create thread pool
	std::vector<std::thread> workers;
	for (int i = 0; i < numThreads; i++) {
		workers.push_back(std::thread(serve, i, dir));
		usleep(30);
	}

	dprintf("ignore closed socket");
	signal(SIGPIPE, SIG_IGN);

	dprintf("create a socket\n");
	int socket = sockCreate();

	dprintf("allow socket to be reused\n");
	int optval = 1;
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	dprintf("convert address\n");
	struct sockaddr_in address = createAddress(port);

	dprintf("bind socket to address\n");
	sockBind(socket, (struct sockaddr*)&address);

	dprintf("listen to socket\n");
	sockListen(socket);

	dprintf("accept connections to socket\n");
	int newSocket;
	while ((newSocket = sockAccept(socket, (struct sockaddr*)&address))) {
		printf("Adding new task: %d\n", newSocket);
		q.push(newSocket);
		qMutex.unlock();
	}

	dprintf("all done. Cleaning up thread pool\n");
	for (auto &thread: workers) {
		thread.join();
	}

}