#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>

#include "sockets.h"
#include "headers.h"
#include "debug.h"
#include "concurrentQueue.h"

ConcurrentQueue q;
std::mutex qMutex;

void serve(int tid, std::string path) {
	while(true) {
		qMutex.lock();
		int sock = q.pop();
		dprintf("serving %d\n", sock);

		char* line;
		do {
			line = GetLine(sock);
			printf("%s\n", line);
		} while (strlen(line) > 0);

		char* str("HTTP/1.1 200 OK\nDate: Mon, 26 Jan 2015 02:12:40 GMT\nServer: Apache/2.2.11 (Unix) PHP/5.3.6 mod_python/3.3.1 Python/2.3.5 mod_fastcgi/2.4.6 DAV/2 SVN/1.4.5 Phusion_Passenger/2.2.5\nAccept-Ranges: bytes\nConnection: close\nContent-Type: text/html\n\n<html>hello</html>\n\n");
		write(sock, str, strlen(str));
		close(sock);
	}
}

int main(int argc, char* argv[]) {
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
	dprintf("accept connections to socket\n");

	int newSocket;
	while (newSocket = sockAccept(socket, (struct sockaddr*)&address)) {
		dprintf("Adding new task: %d\n", newSocket);
		q.push(newSocket);
		qMutex.unlock();
	}

	dprintf("all done. Cleaning up thread pool\n");

	//clean up thread pool
	for (auto &thread: workers) {
		thread.join();
	}

}