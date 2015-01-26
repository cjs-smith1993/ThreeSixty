#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>

#include "sockets.h"
#include "debug.h"
#include "concurrentQueue.h"

ConcurrentQueue q;
std::mutex qMutex;

void serve(int tid, std::string path) {
	while(true) {
		qMutex.lock();
		dprintf("serving\n");
	}
}

int main(int argc, char* argv[]) {
	qMutex.lock();

	//parse arguments
	if (argc < 4) {
		dprintf("Usage: server <port number> <num threads> <dir>\n");
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