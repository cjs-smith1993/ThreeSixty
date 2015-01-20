#include "headers.h"

bool isWhitespace(char c) {
	switch (c)
	{
		case '\r':
		case '\n':
		case ' ':
		case '\0':
		return true;
		default:
		return false;
	}
}

void chomp(char *line) {
	int len = strlen(line);
	while (isWhitespace(line[len]))
	{
		line[len--] = '\0';
	}
}

char * GetLine(int fds) {
	char tline[MAX_MSG_SZ];
	char *line;

	int messagesize = 0;
	int amtread = 0;
	while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
	{
		if (amtread > 0)
			messagesize += amtread;
		else
		{
			perror("Socket Error is:");
			fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
			exit(2);
		}
		if (tline[messagesize - 1] == '\n')
			break;
	}
	tline[messagesize] = '\0';
	chomp(tline);
	line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
	strcpy(line, tline);
	return line;
}

void UpcaseAndReplaceDashWithUnderline(char *str) {
	int i;
	char *s;

	s = str;
	for (i = 0; s[i] != ':'; i++)
	{
		if (s[i] >= 'a' && s[i] <= 'z')
			s[i] = 'A' + (s[i] - 'a');

		if (s[i] == '-')
			s[i] = '_';
	}
}

char *FormatHeader(char *str, char *prefix) {
	char *result = (char *)malloc(strlen(str) + strlen(prefix));
	char* value = strchr(str,':') + 2;
	UpcaseAndReplaceDashWithUnderline(str);
	*(strchr(str,':')) = '\0';
	sprintf(result, "%s%s=%s", prefix, str, value);
	return result;
}

void GetHeaderLines(std::vector<char *> &headerLines, int skt, bool envformat) {
	char *line;
	char *tline;

	tline = GetLine(skt);
	while(strlen(tline) != 0) {
		if (strstr(tline, "Content-Length") || strstr(tline, "Content-Type")) {
			if (envformat) {
				char* str;
				strcpy(str, "");
				line = FormatHeader(tline, str);
			}
			else {
				line = strdup(tline);
			}
		}
		else {
			if (envformat) {
				char* str;
				strcpy(str, "HTTP_");
				line = FormatHeader(tline, str);
			}
			else {
				line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
				sprintf(line, "%s", tline);
			}
		}
		headerLines.push_back(line);
		free(tline);
		tline = GetLine(skt);
	}
	free(tline);
}