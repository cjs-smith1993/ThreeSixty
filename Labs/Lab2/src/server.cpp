#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>
#include <sys/stat.h>

#include "sockets.h"
#include "headers.h"
#include "debug.h"
#include "concurrentQueue.h"

#define LINE_LENGTH 50
#define BUFFER_LENGTH 256

ConcurrentQueue q;
std::mutex qMutex;

void serve(int tid, std::string path) {
	while(true) {
		qMutex.lock();
		int sock = q.pop();
		dprintf("serving %d\n", sock);

		char cmd[LINE_LENGTH];
		char URI[LINE_LENGTH];
		double version;

		//read the request line
		char* line = GetLine(sock);
		sscanf(line, "%s /%s HTTP/%lf", cmd, URI, &version);
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

		if (S_ISREG(filestat.st_mode)) {
			dprintf("Found a file\n");
			fileLength = filestat.st_size;
		}
		else if (S_ISDIR(filestat.st_mode)) {
			//TODO
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

		if (status != 200) {
			char blank[] = "\r\n";
			write(sock, blank, strlen(blank));
			close(sock);
			continue;
		}

		//write the Content-Type line
		const char* fileType = strchr(URI, '.');
		const char* type;
		if (strcmp(fileType, ".html") == 0) {
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
			dprintf("Unknown file type\n");
			char blank[] = "\r\n";
			write(sock, blank, strlen(blank));
			close(sock);
			continue;
		}
		char contentTypeLine[LINE_LENGTH];
		sprintf(contentTypeLine, "Content-Type: %s\r\n", type);
		write(sock, contentTypeLine, strlen(contentTypeLine));
		dprintf("%s", contentTypeLine);

		//write the Content-Length line
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
		dprintf("about to read from %s\n", URI);
		FILE* file = fopen(URI, "r");
		if (!file) {
			printf("WTF this can't happen\n");
		}
		char buf[BUFFER_LENGTH];
		while (fgets(buf, BUFFER_LENGTH, file)) {
			write(sock, buf, strlen(buf));
			dprintf("%s", buf);
		}
		fclose(file);

		// char* str("HTTP/1.1 200 OK\nDate: Mon, 26 Jan 2015 02:12:40 GMT\nServer: Apache/2.2.11 (Unix) PHP/5.3.6 mod_python/3.3.1 Python/2.3.5 mod_fastcgi/2.4.6 DAV/2 SVN/1.4.5 Phusion_Passenger/2.2.5\nAccept-Ranges: bytes\nConnection: close\nContent-Type: text/html\n\n<html>hello</html>\n\n");
		// write(sock, str, strlen(str));
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