#include "server.h"

ConcurrentQueue q;
std::mutex qMutex;

void trimURI(char URI[], char trimmedURI[]) {
	strcpy(trimmedURI, URI);
	char* p = strchr(trimmedURI, '?');
	if (p != NULL) {
		*p = '\0';
	}
}

void canonicalizeURI(std::string root, char URI[]) {
	std::string rootCopy = root.c_str();

	if (rootCopy[0] != '/' && root[0] != '.') {
		rootCopy = "./" + rootCopy;
	}
	if (rootCopy.length() > 1 && rootCopy[rootCopy.length()-1] == '/') {
		rootCopy = rootCopy.substr(0, rootCopy.length()-1);
	}
	if (URI[strlen(URI)-1] == '/') {
		URI[strlen(URI)-1] = '\0';
	}

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
		}
		else if (S_ISDIR(filestat.st_mode)) {
			// check for index.html page
			char indexURI[LINE_LENGTH];
			sprintf(indexURI, "%s/index.html", URI);

			stat(indexURI, &filestat);
			if (S_ISREG(filestat.st_mode)) { // use the index.html page
				sprintf(URI, "%s", indexURI);
			}
			else { //output a directory listing
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
	const char* fileType = strrchr(URI, '.');
	const char* type;

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

int generateDirectory(int rootPathLength, char URI[], char buf[]) {
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
			strcat(buf, URI+rootPathLength);
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

	// write a blank line
	char blank[] = "\r\n";
	write(sock, blank, strlen(blank));
	dprintf("%s\n", blank);
}

void writeFile(int sock, char URI[], int fileLength) {
	FILE* file = fopen(URI, "r");
	int fd = fileno(file);
	off_t* offset;
	if(_sendfile(sock, fd, NULL, fileLength) < 0) {
		dprintf("Error writing file to socket: %d", errno);
	}
	fclose(file);
}

void writeFile(int sock, char dirBuf[]) {
	write(sock, dirBuf, strlen(dirBuf));
}

void runCGI(int sock, std::string requestMethod, char URI[], std::vector<char *> headerLines) {
	dprintf("running CGI script\n");

	// pipes between parent and child
	int ServeToCGIpipefd[2];
	int CGIToServepipefd[2];
	pipe(ServeToCGIpipefd);
	pipe(CGIToServepipefd);

	int pid = fork();
	if (pid == 0) { // child process

		// set up pipes between parent and child
		close(ServeToCGIpipefd[1]);		// close the write side of the pipe from the server
		dup2(ServeToCGIpipefd[0], 0);	// dup the pipe to stdin
		close(CGIToServepipefd[0]);		// close the read side of the pipe to the server
		dup2(CGIToServepipefd[1], 1);	// dup the pipe to stdout

		// create args array
		int numArgs = headerLines.size();
		char* args[1];
		args[0] = NULL;

		// create env array
		int minNumEnv = 4+numArgs;
		int numEnv = minNumEnv; // at least 4 for general CGI vars + args + NULL
		// for (char **env = environ; *env != NULL; env++) {
		// 	numEnv++;
		// }
		char* env[numEnv+1];

		char queryString[LINE_LENGTH];
		if (strchr(URI, '?') == NULL) {
			queryString[0] = '?';
		}
		else {
			sprintf(queryString, "%s", strchr(URI, '?'));
		}

		char env_interface[LINE_LENGTH];
		char env_requestURI[LINE_LENGTH];
		char env_requestMethod[LINE_LENGTH];
		char env_queryString[LINE_LENGTH];

		sprintf(env_interface, "GATEWAY_INTERFACE=%s", "CGI/1.1");
		sprintf(env_requestURI, "REQUEST_URI=%s", URI);
		sprintf(env_requestMethod, "REQUEST_METHOD=%s", requestMethod.c_str());
		sprintf(env_queryString, "QUERY_STRING=%s", ((char*)queryString)+1);

		env[0] = env_interface;
		env[1] = env_requestURI;
		env[2] = env_requestMethod;
		env[3] = env_queryString;

		for (int i = 0; i < numArgs; i++) {
			env[4+i] = headerLines[i];
		}

		// for (int i = minNumEnv; i < numEnv; i++) {
		// 	env[i] = environ[i-minNumEnv];
		// }

		env[numEnv] = NULL;

		// trim the URI before executing the script name
		char trimmedURI[LINE_LENGTH];
		trimURI(URI, trimmedURI);

		execve(trimmedURI, args, env);
	}
	else { // parent process
		close(ServeToCGIpipefd[0]);		// close the read side of the pipe to the CGI script
		close(CGIToServepipefd[1]);		// close the write side of the pipe from the CGI script

		if (strcmp(requestMethod.c_str(), "POST") == 0) {
			// get content length
			int contentLength = -1;
			for (int i = 0; i < headerLines.size(); i++) {
				std::string searchString = "CONTENT_LENGTH=";
				std::string header = headerLines[i];
				int idx = header.find(searchString);
				if (idx >= 0) {
					std::string sub = header.substr(searchString.length());
					contentLength = atoi(sub.c_str());
				}
			}

			// read POST data from request
			char buf[contentLength+1];
			int totalNumRead = 0;
			int numRead = 0;
			while (totalNumRead < contentLength) {
				numRead = read(sock, buf+totalNumRead, contentLength-totalNumRead);
				totalNumRead += numRead;
				if (numRead == 0) {
					break;
				}
			}
			buf[contentLength] = '\0';

			// send data to CGI script
			write(ServeToCGIpipefd[1], buf, contentLength);
		}

		// read from the child and write to the socket
		char buf[BUFFER_LENGTH];
		int totalNumRead = 0;
		int numRead = 0;
		while ((numRead = read(CGIToServepipefd[0], buf+totalNumRead, LINE_LENGTH))) {
			totalNumRead += numRead;
		}
		buf[totalNumRead] = '\0';
		// dprintf("%s", buf);
		write(sock, buf, strlen(buf));

		// close the pipes
		close(ServeToCGIpipefd[1]);
		close(CGIToServepipefd[0]);

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

		int optval = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		char requestMethod[LINE_LENGTH];
		char URI[LINE_LENGTH];
		memset(URI, 0, LINE_LENGTH);
		double version;

		// read the request line
		char* line = GetLine(sock);
		sscanf(line, "%s %s HTTP/%lf", requestMethod, URI, &version);

		// normalize root path and URI
		int rootPathLength = rootPath.length();
		canonicalizeURI(rootPath, URI);
		dprintf("Command: %s\nURI: %s\nHTTP version: %lf\n", requestMethod, URI, version);

		// read the rest of the request headers
		std::vector<char*> headerLines;
		GetHeaderLines(headerLines, sock, true);

		// for (int i = 0; i < headerLines.size(); i++) {
		// 	dprintf("%s\n", headerLines[i]);
		// }

		// trim the query string from URI
		char trimmedURI[LINE_LENGTH];
		trimURI(URI, trimmedURI);

		// write the status line
		int status = getStatus(sock, trimmedURI);
		std::string message = getMessage(status);
		bool isDir = isDirectory(trimmedURI);
		writeStatusLine(sock, status, message);

		// write the Content-Type line
		std::string type = getContentType(sock, trimmedURI);
		if (strcmp(type.c_str(), "dynamic") == 0) { // handle CGI scripts separately
			runCGI(sock, requestMethod, URI, headerLines);
			continue;
		}
		writeContentTypeLine(sock, type);

		// write the Content-Length line
		char dirBuf[BUFFER_LENGTH];
		int fileLength = isDir ? generateDirectory(rootPathLength, trimmedURI, dirBuf) : getContentLength(sock, trimmedURI);
		writeContentLengthLine(sock, fileLength);

		// write the rest of the response headers and a blank line
		writeEndOfHeaders(sock);

		// write the file
		isDir ? writeFile(sock, dirBuf) : writeFile(sock, trimmedURI, fileLength);

		// close the socket
		shutdown(sock, SHUT_RDWR);
		close(sock);

		dprintf("----------------\n");
	}
}

int main(int argc, char* argv[]) {
	// lock the signal semaphore
	qMutex.lock();

	// parse the arguments
	if (argc < 4) {
		printf("Usage: server <port number> <num threads> <dir>\n");
		exit(0);
	}

	int port = atoi(argv[1]);
	int numThreads = atoi(argv[2]);
	char* dir = argv[3];

	// create the thread pool
	std::vector<std::thread> workers;
	for (int i = 0; i < numThreads; i++) {
		workers.push_back(std::thread(serve, i, dir));
		usleep(30);
	}

	signal(SIGPIPE, SIG_IGN);

	// create a socket
	int socket = sockCreate();

	int optval = 1;
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	// convert address
	struct sockaddr_in address = createAddress(port);

	// bind socket to address
	sockBind(socket, (struct sockaddr*)&address);

	// listen to socket
	sockListen(socket);

	dprintf("setup complete\n----------------\n");

	// accept connections to socket
	int newSocket;
	while ((newSocket = sockAccept(socket, (struct sockaddr*)&address))) {
		printf("adding new task on socket %d\n", newSocket);
		q.push(newSocket);
		qMutex.unlock();
	}

}