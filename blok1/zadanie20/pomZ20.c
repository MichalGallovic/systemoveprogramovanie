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
	sigaddset(&set, SIGUSR2);
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	char *line = NULL;
	ssize_t read;
	size_t len = 0;
	//printf("%s\n",argv[1]);
	//printf("pid pocesu %d\n", getpid());
	//printf("pid druheho procesu %d\n",atoi(argv[2]));
	file = fopen(argv[1], "r");
	if (file) {
	//	printf("nejej\n");
		while(1){
				kill(atoi(argv[2]), SIGUSR1);
				sigwait(&set, &sig_number);
				if((read = getline(&line, &len, file)) != -1) 
					printf("%s", line);
				else exit(EXIT_SUCCESS);
		}
			
		fclose(file);
	} 
	return EXIT_SUCCESS;
}
