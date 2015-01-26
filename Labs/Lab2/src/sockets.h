#ifndef SOCKETS_H
#define SOCKETS_H

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "debug.h"

#define MAX_BACKLOG 5

sockaddr_in createAddress(int hostPort);
int sockCreate();
void sockBind(int socket, const struct sockaddr *address);
void sockListen(int socket);
int sockAccept(int socket, struct sockaddr* address);

#endif // SOCKETS_H
