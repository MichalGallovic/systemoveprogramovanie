// argumenty pisem za sebou, nakonci dame null
execlp("ls","ls","-la",NULL);

// exec a pole argumentov
char *argv[] = {
	"test",   // The first argument, by convention, should point to the filename associated with the file being executed.
	"-la",
	NULL
};
execv("/bin/ls",argv);
execvp("ls",argv);

