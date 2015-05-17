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
	//printf("nazov textaku %s\n",argv[1]);
//	printf("moj pid %d\n",getpid());
//	printf("pid druheho pocesu %d\n",atoi(argv[2]));
	file = fopen(argv[1], "r");
	if (file) {
		
	//	printf("som v ife\n");
		while(1){
	//		printf("som vo vajle\n");
			sleep(1);
			sigwait(&set, &sig_number);
	//		printf("som za sigvejtom\n");
			if((read = getline(&line, &len, file)) != -1) 
				printf("%s", line);
			else exit(EXIT_SUCCESS);
			kill(atoi(argv[2]), SIGUSR2);
		}
			
		fclose(file);
	} 

	return EXIT_SUCCESS;
}
