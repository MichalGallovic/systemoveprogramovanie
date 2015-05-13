#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>


int main(int argc, char * argv[]) {
	
	FILE *file;
	
	int sig_number;
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	char *line = NULL;
	ssize_t read;
	size_t len = 0;

	printf("pid pocesu %d\n", getpid());
	file = fopen(argv[1], "r");
	if (file) {
		while(1){
				kill(atoi(argv[2]), SIGUSR2);
				sigwait(&set, &sig_number);
				if((read = getline(&line, &len, file)) != -1) 
					printf("%s", line);
		}
			
		fclose(file);
	}

	return EXIT_SUCCESS;
}
