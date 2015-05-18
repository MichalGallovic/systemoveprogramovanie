#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include "maped_mem.h"

typedef enum {
	UNSET = 0,
	SET = 1
} OPTION;

typedef struct {
	OPTION h;
	OPTION n;
	char * nproc;
	int n_proc;
	OPTION f;
	char * file_name;
} ARGS;

int quit = 1;
 
void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage: parametre [-h] [-n | --n_proc <N>] -f | -file <file>\n");
	fprintf(stream, "Zadanie 2-01\n");
	fprintf(stream, "Prepinace:\n");
	fprintf(stream, " -h, --help   vypise help\n");
	fprintf(stream, " -n, --n_proc pocet procesov\n");
	fprintf(stream, " -f, --file nazov suboru, do ktoreho sa to ma zapisat\n");
	exit(exitCode);
}

void parseArgs(int argc, char * argv[], ARGS * args) {
	int opt;

	args->h = UNSET;
	args->n = UNSET;
	args->nproc = NULL;
	args->n_proc = 4;
	args->f = UNSET;
	args->file_name = NULL;
	
	static struct option long_options[] = {
                   						  {"help", 0, NULL, 'h'},
                   						  {"n_proc", 1, NULL, 'n'},
                   						  {"file", 1, NULL, 'f'},
                   						  {0, 0, 0, 0}
               							  };
	int option_index = 0;

	do {
		opt = getopt_long(argc, argv, ":hn:f:", long_options, &option_index);
		switch (opt) {
		case 'h':
			args->h = SET;
			printHelpAndExit(stderr, EXIT_SUCCESS);
			break;
		case 'n':
			args->n = SET;
			args->nproc = optarg;
			break;
		case 'f':
			args->f = SET;
			args->file_name = optarg;
			break;
		case '?': 	
			fprintf(stderr,"Neznama volba -%c\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		case ':': 	
			fprintf(stderr, "Nebol zadany argument prepinaca '-%c'\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		default:
			break;
		}
		
	} while(opt != -1);

	while(optind < argc ) {
		printf("Debug:    non-option ARGV-element: %s\n", argv[optind++]);
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
}

void validateArgs(ARGS * args) {
	if(args->n == SET){
		if(sscanf(args->nproc, "%d", &args->n_proc) <= 0 ) {
			fprintf(stderr, "Argument prepinaca -n nie je cislo!\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
		}
		if(args->n_proc < 4){
			fprintf(stderr, "Argument prepinaca -n musi byt vacsi ako 4\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
		}	
	}
	if(args->f != SET){
		fprintf(stderr, "Prepinac -f je povinny\n");
		printHelpAndExit(stderr, EXIT_FAILURE);	
	}
	if(args->f == SET && args->file_name == NULL){
		fprintf(stderr, "Argument prepinaca -f nebol zadany\n");
		printHelpAndExit(stderr, EXIT_FAILURE);	
	}
}

void shuffle(int *array, size_t n){
    if (n > 1) {
        size_t i;
	for (i = 0; i < n - 1; i++) {
	  size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
	  int t = array[j];
	  array[j] = array[i];
	  array[i] = t;
	}
    }
}

void ChildSignalHandler(){
	quit = 0;
}

void doWork(int position, int position1){
	char buf[4096];
	char buf1[4096];
	char temp[20];
	struct sigaction sa;
	sigset_t sigsetMask;
	
	buf[0] = '\0';
	
	sigfillset(&sigsetMask);
	sigdelset(&sigsetMask, SIGUSR1);
	sigprocmask( SIG_BLOCK, &sigsetMask, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &ChildSignalHandler;
	CHECK(sigaction(SIGUSR1, &sa, NULL) ==  0);

	while(quit){
		lockSem(position);
		read_((char*)&buf1); //precita obsah pamate
		sprintf((char*)&temp, "%u\n", getpid()); ///vytvori string z PID
		// fprintf(stderr, "PID:%s", temp);
		strcat((char*)&buf, (char*)&buf1); //spoji dokopy
		// fprintf(stderr, "@Maped:%s", buf);
		strcat((char*)&buf, (char*)&temp);
		// fprintf(stderr, "2Maped:%s", buf);
		write_((char*)&buf, strlen(buf));
		sleep(1);
		unlockSem(position1);
		buf[0] = '\0'; //premazanie
	}
	fprintf(stderr, "Killed %u\n", getpid());
}

//vracia poziciu nasledujuceho v poli, ak je posledny tak prveho
int getNext(int* array, int position, int max){
	if(position == max)
		return array[0];
	else 
		return array[position];
}
void createProc(int number, pid_t* pid_array){
	int i = 0;
	pid_t pid;
	int *array;
	
	//Preparing random order squence
	array = (int*)malloc(sizeof(int)*number);

	for(i = 0; i < number; ++i)
		array[i] = i;
	shuffle(array, number);

	unlockSem(0); //aby mohol prvy bezat
	for(i = 0; i < number; ++i){
		switch(pid = fork()){
			case 0:
      			doWork(array[i], getNext(array, i + 1, number));
				exit(EXIT_SUCCESS);
    		case -1:
      			perror("fork");
      			exit(EXIT_FAILURE);
    		default:
				pid_array[i] = pid;
      			break;
		}
	}
	free(array);
}

int main(int argc, char * argv[]){
 	ARGS args;
	pid_t *pid;
	int i = 0;
	char buf[4096];

	parseArgs(argc, argv, &args);
	validateArgs(&args);

	init(args.n_proc, args.file_name);

	pid = (pid_t*)malloc(sizeof(pid_t) * args.n_proc); 	
	createProc(args.n_proc, pid);

	//KILLOVANIE deti
	sigset_t set;
	int sig_num;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	CHECK(sigprocmask(/*SIG_BLOCK*/ SIG_SETMASK, &set, NULL) == 0);
	CHECK(sigwait(&set, &sig_num) == 0);

	for(i = 0; i < args.n_proc; i++){
		CHECK(kill(pid[i], SIGUSR1) == 0 );
	}

	for(i = 0; i < args.n_proc; i++){
		CHECK(wait(NULL) != -1);	
	}

	read_((char*)&buf);
	fprintf(stderr, "%s\n", buf);
	
	teardown(0);
	free(pid);
	printf("detske procesy ukoncene\n");
	return EXIT_SUCCESS;
}
