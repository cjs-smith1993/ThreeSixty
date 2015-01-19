#ifndef HEADERS_H
#define HEADERS_H

#include <string.h>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

#define MAX_MSG_SZ		1024

bool isWhitespace(char c);
void chomp(char *line);
char * GetLine(int fds);
void UpcaseAndReplaceDashWithUnderline(char *str);
char *FormatHeader(char *str, char *prefix);
void GetHeaderLines(std::vector<char *> &headerLines, int skt, bool envformat);

#endif // HEADERS_H
