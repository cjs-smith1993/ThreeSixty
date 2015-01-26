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

#define BUFFER_LENGTH 256

ConcurrentQueue q;
std::mutex qMutex;

void serve(int tid, std::string path) {
	while(true) {
		qMutex.lock();
		int sock = q.pop();
		dprintf("serving %d\n", sock);

		char cmd[3];
		char URI[20];
		double version;

		//read the request line
		char* line = GetLine(sock);
		sscanf(line, "%s %s HTTP/%lf", cmd, URI, &version);
		dprintf("Command: %s\nURI: %s\nHTTP version: %lf\n\n", cmd, URI, version);

		//read the rest of the request headers
		while (strlen(line) > 0) {
			dprintf("%s\n", line);
			line = GetLine(sock);
		}

		//write the status line
		char status[] = "HTTP/1.1 200 OK\r\n";
		write(sock, status, strlen(status));
		dprintf("%s", status);

		//get the Content-Type
		char contentType[] = "Content-Type: text/html\r\n";
		write(sock, contentType, strlen(contentType));
		dprintf("%s", contentType);

		//get the Content-Length
		long int fileLength;
		struct stat filestat;
		stat("test.html", &filestat);
		if (S_ISREG(filestat.st_mode)) {
			fileLength = filestat.st_size;
			dprintf("%ld\n", fileLength);
		}
		char contentLength[25];
		sprintf(contentLength, "Content-Length: %ld\n", fileLength);
		write(sock, contentLength, strlen(contentLength));
		dprintf("%s", contentLength);

		//write the rest of the response headers
		char other[] = "Connection: close\r\n\r\n";
		write(sock, other, strlen(other));
		dprintf("%s", other);

		//write the file
		FILE* file = fopen("test.html", "r");
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