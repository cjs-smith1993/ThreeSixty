#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>
#include <sys/stat.h>
#include <dirent.h>

#include "sockets.h"
#include "headers.h"
#include "debug.h"
#include "concurrentQueue.h"

#define LINE_LENGTH 50
#define BUFFER_LENGTH 500

ConcurrentQueue q;
std::mutex qMutex;

void generateDirectory(char URI[], char buf[]) {
	dprintf("%s\n", URI);
	strcpy(buf, "<html>\r\n<body>\r\n<h1>Index of ");
	strcpy(buf, URI);
	strcpy(buf, "</h1>\r\n<ul>\r\n");

	DIR* d;
	struct dirent* dir;
	d = opendir(URI);
	if (d) {
		while ((dir = readdir(d))) {
			dprintf("%s\n", dir->d_name);
			strcat(buf, "<li><a href=\"");
			if (dir->d_name[0] != '.') {
				strcat(buf, URI);
				strcat(buf, "/");
			}
			strcat(buf, dir->d_name);
			strcat(buf, "\"> ");
			strcat(buf, dir->d_name);
			strcat(buf, "</a></li>\r\n");
		}

		closedir(d);
	}

	strcat(buf, "</ul>\r\n</body>\r\n</html>");

}

void serve(int tid, std::string path) {
	while(true) {
		qMutex.lock();
		int sock = q.pop();
		dprintf("serving %d\n", sock);

		bool directory = false;
		char dirBuf[BUFFER_LENGTH];

		char cmd[LINE_LENGTH];
		char URI[LINE_LENGTH];
		memset(URI, 0, LINE_LENGTH);
		double version;

		//read the request line
		char* line = GetLine(sock);
		sscanf(line, "%s %s HTTP/%lf", cmd, URI, &version);
		memmove(URI+1, URI, strlen(URI)); //prepend leading '.'
		URI[0] = '.';
		dprintf("\n----------------\nCommand: %s\nURI: %s\nHTTP version: %lf\n\n", cmd, URI, version);

		//read the rest of the request headers
		while (strlen(line) > 0) {
			dprintf("%s\n", line);
			line = GetLine(sock);
		}

		//write the status line
		int status = 200;
		char message[LINE_LENGTH];
		sprintf(message, "OK");
		long int fileLength;
		struct stat filestat;
		stat(URI, &filestat);

		if (fopen(URI, "r") && S_ISREG(filestat.st_mode)) {
			dprintf("Found a file\n");
		}
		else if (S_ISDIR(filestat.st_mode)) {
			dprintf("Found a directory\n");
			//check for index.html page
			char indexURI[LINE_LENGTH];
			sprintf(indexURI, "%s/index.html", URI);

			stat(indexURI, &filestat);
			if (S_ISREG(filestat.st_mode)) { //use the index.html page
				sprintf(URI, "%s", indexURI);
				dprintf("%s\n", URI);
			}
			else { //output a directory listing
				dprintf("%s is not an index file\n", indexURI);
				directory = true;
				generateDirectory(URI, dirBuf);
				fileLength = strlen(dirBuf);
			}
		}
		else {
			status = 404;
			sprintf(message, "Not Found");
			sprintf(URI, "404.html");
		}

		char statusLine[LINE_LENGTH];
		sprintf(statusLine, "HTTP/1.1 %d %s\r\n", status, message);
		write(sock, statusLine, strlen(statusLine));
		dprintf("%s", statusLine);

		//write the Content-Type line
		const char* fileType = strchr(URI+1, '.'); //ignore first '.'
		const char* type;
		dprintf("%s\n", fileType);
		if (!fileType || strcmp(fileType, ".html") == 0) {
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
		else {
			type = "text/plain";
			dprintf("Unknown file type\n");
		}
		char contentTypeLine[LINE_LENGTH];
		sprintf(contentTypeLine, "Content-Type: %s\r\n", type);
		write(sock, contentTypeLine, strlen(contentTypeLine));
		dprintf("%s", contentTypeLine);

		//write the Content-Length line
		stat(URI, &filestat);
		if (fopen(URI, "r") && S_ISREG(filestat.st_mode)) {
			fileLength = filestat.st_size;
		}
		char contentLength[LINE_LENGTH];
		sprintf(contentLength, "Content-Length: %ld\r\n", fileLength);
		write(sock, contentLength, strlen(contentLength));
		dprintf("%s", contentLength);

		//write the rest of the response headers
		char other[] = "Connection: close\r\n";
		write(sock, other, strlen(other));
		dprintf("%s", other);

		//write a blank line
		char blank[] = "\r\n";
		write(sock, blank, strlen(blank));
		dprintf("%s\n", blank);

		//write the file
		if (!directory) {
			dprintf("about to read from %s\n", URI);
			FILE* file = fopen(URI, "r");
			char buf[BUFFER_LENGTH];
			while (fgets(buf, BUFFER_LENGTH, file)) {
				write(sock, buf, strlen(buf));
				dprintf("%s", buf);
			}
			fclose(file);
		}
		else {
			dprintf("about to read directory listing");
			write(sock, dirBuf, strlen(dirBuf));
		}

		close(sock);
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
	}

	dprintf("create a socket\n");
	int socket = sockCreate();

	dprintf("convert address\n");
	struct sockaddr_in address = createAddress(port);

	dprintf("bind socket to address\n");
	sockBind(socket, (struct sockaddr*)&address);

	dprintf("listen to socket\n");
	sockListen(socket);

	dprintf("allow socket to be reused\n");
	int optval = 1;
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	dprintf("accept connections to socket\n");
	int newSocket;
	while (newSocket = sockAccept(socket, (struct sockaddr*)&address)) {
		dprintf("Adding new task: %d\n", newSocket);
		q.push(newSocket);
		qMutex.unlock();
	}

	dprintf("all done. Cleaning up thread pool\n");
	for (auto &thread: workers) {
		thread.join();
	}

}