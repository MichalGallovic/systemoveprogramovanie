#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include "utility.h"

void print(char * formatString, ...)
{
	const int pidStrLength = 11;
	char pidStr[pidStrLength+1];

	int offset;
	memset(pidStr,' ',pidStrLength);
	pidStr[pidStrLength]   = '\0';
	
	offset = (int)getpid() % pidStrLength;
	pidStr[offset] = 'a'+offset;

	va_list args;
	va_start(args, formatString);
	printf("%5d %s: ", getpid(), pidStr);
	vprintf(formatString, args);
	va_end(args);
}
