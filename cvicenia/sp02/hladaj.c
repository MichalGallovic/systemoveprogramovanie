#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	char *args[] = {
		"find",
		".",
		"-name",
		"*.c",
		NULL,
	};
	
	execvp(args[0], args);
	perror("exec");	
	return EXIT_FAILURE;
}

	
