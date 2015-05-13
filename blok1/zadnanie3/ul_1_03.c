#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define BUFFER_SIZE 25
#define READ 0
#define WRITE 1

typedef enum {
	UNSET = 0,
	SET = 1
} OPTION;

typedef struct {
	OPTION p;
	char* pArg;
	OPTION f;
	char* fArg;
} ARGS;


void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage parameters [-c | --count]\n");
	fprintf(stream, " -c, --count outputs number of regular files in defined directory\n");
	//exit(exitCode);
}
/*
static void default_signal_handler(int signo){
	printf("mam signal");
	exit(EXIT_SUCCESS);
}
*/
void parseArgs(int argc, char * argv[], ARGS * args){
	int opt;

	struct option long_options[] = {
		{"program", required_argument, NULL, 'p'},
		{"file", required_argument, NULL, 'f'},
		{NULL, 0, NULL, 0}
	};

	args->p = UNSET;
	args->pArg = NULL;
	args->f = UNSET;
	args->fArg = NULL;

	printf("argc: %d\n", argc);	

	do {
		opt = getopt_long(argc, argv, "p:f:", long_options, NULL);
		printf("Debug: opt='%c'\n", opt);
		switch (opt) {
       		case 'p':
				args->p = SET;
				args->pArg = optarg;
				break;
			case 'f':
				args->f = SET;
				args->fArg = optarg;
				break;
	       	case '?':
    	       	fprintf(stderr, "Chyba: neznama volba '%c'\n",optopt);
				printHelpAndExit(stderr, EXIT_FAILURE);
				break; 
		}
		printf("optind: %d\n", optind);
	}while(opt != -1);

	while(optind < argc){
		printf("Debug:	non-option ARGV-element: %s\n", argv[optind++]);
	}
}

void outFile(char* filename, int pid, sigset_t set, int signumber){
	char * line = NULL;
	FILE *fp;

	size_t len = 0;
	ssize_t read;

	fp = fopen(filename, "r");

	if(fp){
		printf("pid kam posielam kill %d\n", pid);
		while(1){
			sigwait(&set, &signumber);
			//kill(pid, SIGUSR1);
			if((read = getline(&line, &len, fp))!=-1){
				printf("%s", line);
				kill(pid, SIGUSR1);
			}else{
				exit(EXIT_SUCCESS);
			}	
		
		}

	}
	
	fclose(fp);
	 
	
}

int main(int argc, char * argv[]) {
	ARGS args;
	parseArgs(argc, argv, &args);

	int parPid = getpid();
	printf("pid: %d\n", parPid);

	sigset_t set;
				int sig_number = 0;
			//	struct sigaction sa_def;
/*
	memset(&sa_def, 0, sizeof(sa_def));
	sa_def.sa_handler = &default_signal_handler;
*/
				sigemptyset(&set);
				sigaddset(&set, SIGUSR2);		
				sigprocmask(SIG_BLOCK, &set, NULL);


	if(args.p == UNSET){
		printf("Argument p je povinný\n");
		return EXIT_FAILURE;
	}else{
		printf("dostal som argument p - %s\n", args.pArg);
		
		int channel[2];

		if(args.f==UNSET){

			pipe(channel);
			pid_t pidC = fork();

			if(pidC==0){
				//execlp("ls", "ls", "-f", (char *) NULL);
				system("find ./");
				printf("Zadajte subor, ktorýc chete prečítať po iadkoch dvakrát\n");
				char input_array[50];
				scanf("%s", input_array);
				
				close(channel[0]);
				write(channel[1], input_array, sizeof(input_array));
				close(channel[1]);
				//args.f = SET;
				//args.fArg = input_array;
				exit(EXIT_SUCCESS);
			}else{
				//kill(pidC, SIGKILL);
				char input_array[50];
				close(channel[1]);
				while( read(channel[0],input_array, sizeof(input_array))==sizeof(input_array) ){
	//close(channel[0]);
					printf("%s\n", input_array);
					args.fArg = input_array;
				}
				args.f = SET;
				close(channel[0]);	
				waitpid(pidC, 0, 0);
			}
		}



		if(args.f == SET){
			printf("dostal som argument f - %s\n", args.fArg);	

			pid_t pid = fork();
		
	
			if (pid==0){

				char s[30];
				sprintf(s, "%d", parPid);

						
				char *argv[] = {"test", args.fArg, s, NULL};
				printf("spostam pom program %d\n", parPid);
				execv(args.pArg, argv);

				//exit(127); 
			}else{
				//waitpid(pid, 0, 0);
				printf("detske pid je %d\n", pid);
				outFile(args.fArg, pid, set, sig_number);

			}			
		}
		
	}
	
	printf("som tu\n");
	while(1){
		sigwait(&set, &sig_number);
		printf("test test\n");
		sleep(1);
	}

	return EXIT_SUCCESS;
}
