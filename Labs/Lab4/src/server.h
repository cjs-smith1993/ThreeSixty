#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>
#include <sys/stat.h>
#include <dirent.h>

#if __APPLE__
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/uio.h>
#else
	#include <sys/sendfile.h>
#endif

#include "sockets.h"
#include "headers.h"
#include "debug.h"
#include "concurrentQueue.h"

#define LINE_LENGTH 50
#define BUFFER_LENGTH 10000

void canonicalizeURI(std::string root, char URI[]);
void generateDirectory(char URI[], char buf[]);
void serve(int tid, std::string rootPath);

int _sendfile(int sock, int fd, off_t* offset, size_t length) {
	#if __APPLE__
		long long longLength = (long long) length;
		long long* newLength = &longLength;
		return sendfile(fd, sock, (off_t) NULL, newLength, NULL, 0);
	#else
		return senfile(sock, fd, offset, length);
	#endif
}


#endif // SERVER_H
