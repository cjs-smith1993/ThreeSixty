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
#include <signal.h>

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

#define LINE_LENGTH 500
#define BUFFER_LENGTH 10000

extern char** environ;

void trimURI(char URI[], char trimmedURI[]);
void canonicalizeURI(std::string root, char URI[]);
bool isDirectory(char URI[]);
std::string getMessage(int status);
int getStatus(int sock, char URI[]);
void writeStatusLine(int sock, int status, std::string message);
std::string getContentType(int sock, char URI[]);
void writeContentTypeLine(int sock, std::string type);
int getContentLength(int sock, char URI[]);
void writeContentLengthLine(int sock, int fileLength);
int generateDirectory(char URI[], char buf[]);
void writeEndOfHeaders(int sock);
void writeFile(int sock, char URI[], int fileLength);
void writeFile(int sock, char dirBuf[]);
void serve(int tid, std::string rootPath);

int _sendfile(int sock, int fd, off_t* offset, size_t length) {
	#if __APPLE__
		long long longLength = (long long) length;
		long long* newLength = &longLength;
		return sendfile(fd, sock, (off_t) NULL, newLength, NULL, 0);
	#else
		return sendfile(sock, fd, offset, length);
	#endif
}


#endif // SERVER_H
