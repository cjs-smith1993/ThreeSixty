#include "sockets.h"

sockaddr_in createAddress(int hostPort) {
	struct sockaddr_in address;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(hostPort);
	address.sin_family = AF_INET;

	return address;
}

int sockCreate() {
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		printf("\nCould not make a socket: %d\n", errno);
		exit(0);
	}
	return sock;
}

void sockBind(int socket, const struct sockaddr *address) {
	if (bind(socket, address, sizeof(*address)) < 0) {
		printf("Could not bind socket to port: %d\n", errno);
		exit(0);
	}
}

void sockListen(int socket) {
	if (listen(socket, MAX_BACKLOG) < 0) {
		printf("Could not listen to socket: %d\n", errno);
		exit(0);
	}
}

int sockAccept(int socket, struct sockaddr* address) {
	socklen_t size = sizeof(address);
	int newSocket = accept(socket, address, &size);
	if (newSocket < 0) {
		printf("Could not accept connection: %d\n", errno);
		exit(0);
	}
	return newSocket;
}