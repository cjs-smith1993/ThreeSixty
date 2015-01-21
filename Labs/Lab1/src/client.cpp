#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "headers.h"

#define SOCKET_ERROR		-1
#define BUFFER_SIZE			100
#define HOST_NAME_SIZE		255

bool debugging;
bool customCount;
int numRequests;
int numResponses;

int makeSocket() {
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == SOCKET_ERROR) {
		printf("\nCould not make a socket\n");
		exit(0);
	}
	return sock;
}

sockaddr_in convertHostToAddress(char* hostName, int hostPort) {
	struct hostent* pHostInfo = gethostbyname(hostName);
	if (!pHostInfo) {
		printf("\nError converting hostname to address\n");
		exit(0);
	}
	long hostAddress;
	memcpy(&hostAddress, pHostInfo->h_addr, pHostInfo->h_length);

	struct sockaddr_in address;
	address.sin_addr.s_addr = hostAddress;
	address.sin_port = htons(hostPort);
	address.sin_family = AF_INET;

	return address;
}

void makeConnection(int sock, struct sockaddr* address) {
	int con = connect(sock, address, sizeof(*address));
	if (con == SOCKET_ERROR) {
		printf("\nCould not connect to host\n");
		exit(0);
	}
}

void sendRequest(int sock, std::string method, std::string URI, std::string version, std::string host) {
	std::string str = method + " "
					+ URI + " "
					+ version
					+ "\r\n"
					+ "Host: " + host
					+ "\r\n\r\n";
	const char* request = str.c_str();

	if (debugging && !customCount) {
		printf("%s\n", request);
	}

	int requestLen = strlen(request);
	int totalNumSent = 0;
	int numSent = 0;
	while (totalNumSent < requestLen) {
		numSent = write(sock, request+totalNumSent, requestLen-totalNumSent);
		if (numSent < 0) {
			printf("\nFailed to write to socket\n");
		}
		totalNumSent+=numSent;
	}
}

bool readResponse(int sock, std::string fileName) {
	std::vector<char*> headerLines;
	GetHeaderLines(headerLines, sock, false);

	bool connectionOpen = true;
	int contentLength = -1;
	int found = -1;
	for (int i = 0; i < headerLines.size(); i++) {
		std::string cur = headerLines[i];
		if ((found = cur.find("Content-Length: ")) >= 0) {
			std::string sub = cur.substr(16);
			contentLength = atoi(sub.c_str());
		}
		if ((found = cur.find("Connection: close")) >= 0) {
			connectionOpen = false;
		}
		if (debugging && !customCount) {
			printf("%s\n", headerLines[i]);
		}
	}

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
	if (!customCount) {
		printf("%s\n", buf);
	}
	std::ofstream response;
	response.open(fileName.c_str());
	response << buf;
	response.close();
	numResponses++;

	return connectionOpen;
};

void closeSocket(int sock) {
	int status = close(sock);
	if (status == SOCKET_ERROR)
	{
		printf("\nCould not close socket\n");
		exit(0);
	}
}

int main(int argc, char* argv[]) {
	char hostName[HOST_NAME_SIZE];
	int hostPort;
	char hostURI[HOST_NAME_SIZE];
	debugging = false;
	numRequests = 1;
	numResponses = 0;

	opterr = 0;
	int c;
	while ((c = getopt(argc, argv, "dc:")) != -1) {
		switch(c) {
			case 'd':
				debugging = true;
				break;
			case 'c':
				customCount = true;
				std::stringstream(optarg) >> numRequests;
				if (numRequests < 1) {
					printf("Option -c requires a positive integer argument\n");
					return 0;
				}
				break;
			case '?':
				if (optopt == 'c') {
					printf("Option -c requires a positive integer argument\n");
				}
				else {
					printf("Unknown option -%c\n", optopt);
				}
				return 0;
		}
	}
	if (argc < 4) {
		printf("\nUsage: download host-name host-port URI\n");
		return 0;
	}
	else {
		strcpy(hostName, argv[argc-3]);
		hostPort=atoi(argv[argc-2]);
		strcpy(hostURI, argv[argc-1]);
	}


	for (int i = 0; i < numRequests; i++) {
		// printf("\nMaking a socket\n");
		int sock = makeSocket();

		// printf("Converting host name into IP address\n");
		struct sockaddr_in address = convertHostToAddress(hostName, hostPort);

		// printf("Connecting to %s on port %d\n", hostName, hostPort);
		makeConnection(sock, (struct sockaddr*)&address);

		sendRequest(sock, "GET", hostURI, "HTTP/1.0", hostName);
		std::stringstream ss;
		ss << "downloads/response";
		ss << i;
		ss << ".html";
		readResponse(sock, ss.str());

		// printf("\nClosing socket\n");
		closeSocket(sock);
	}

	if (customCount) {
		printf("Successfully downloaded file %d time(s)\n", numResponses);
	}

}
