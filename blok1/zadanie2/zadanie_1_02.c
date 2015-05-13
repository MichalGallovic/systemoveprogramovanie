#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define FIFO_NAME "fifo-sp02"

char file_path[PATH_MAX + 1];
int count_ = 0;
int n_days;

void child(){
	int fifo;
        fprintf(stderr, "Zadajte cestu k priecinku:");
        scanf("%100s", (char*)&file_path);

	if(kill(getppid(), SIGUSR1) != 0 ){
		perror("kill");
		exit(EXIT_FAILURE);
	}
	//FIFO
	umask(0);
	if(	mkfifo(FIFO_NAME, 0660) == -1 && errno != EEXIST){
		perror("mkfifo");
		exit(EXIT_FAILURE);
	}

	if((fifo = open(FIFO_NAME, O_WRONLY)) == -1 ){
		perror("open");
		exit(EXIT_FAILURE);
	}
	if(unlink(FIFO_NAME) != 0){
		perror("unlink");
		exit(EXIT_FAILURE);
	} 

	write(fifo, &file_path, sizeof(file_path));

	if(close(fifo) != 0) {	
		perror("close");
		exit(EXIT_FAILURE);
	}
}

void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage: parametre [-h | --help] <pocet_dni>  [-c | --count] [<dir>]\n");
	fprintf(stream, "Program vypise mena vsetkych regularne subory v adresarovej strukture, ku ktorym bol pristup za menej ako <pocet_dni> dni. Ak je definovany prepinac, zobrazi iba ich pocet\n");
	exit(exitCode);
}

void getPath(){
	char buffer[50];
	fprintf(stderr, "Zadajte cestu k novemu priecinku:");
	scanf("%49s", (char*)&buffer);
	strncpy(file_path, buffer, sizeof(file_path));
}

void getPathFromChild(){
	int fifo;
	pid_t pid;
	int sig_num;
	sigset_t set;
	
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	if(sigprocmask(SIG_SETMASK, &set, NULL) != 0){
		perror("sigprocmask");
		 exit(EXIT_FAILURE);
	}

	pid = fork();
  	switch(pid){
    		case 0:
      			child(file_path);
			exit(EXIT_SUCCESS);
    		case -1:
      			perror("fork");
      			exit(EXIT_FAILURE);
    		default:
			if(sigwait(&set, &sig_num) != 0){
				perror("sigwait");
				exit(EXIT_FAILURE);
			}
      			break;
  	}
	//FIFO
	umask(0); 
	if(	mkfifo(FIFO_NAME, 0660) == -1 &&  errno != EEXIST){
		perror("mkfifo");
		exit(EXIT_FAILURE);
	}

	if((fifo = open(FIFO_NAME, O_RDONLY)) == -1) {
                perror("open");
                exit(EXIT_FAILURE);
        }

	if(read(fifo, &file_path, sizeof(file_path)) != sizeof(file_path)) {
		fprintf(stderr, "Error occured while reading from FIFO");
		exit(EXIT_FAILURE);	
	}
        if(close(fifo) != 0) {
                perror("close");
                exit(EXIT_FAILURE);
        }

}

void parseArgs(int argc, char * argv[]) {
	int opt;

	static struct option long_options[] = {    		  {"help", 0, NULL, 'h'},
                   						  {"count", 0, NULL, 'c'},
                   						  {0, 0, 0, 0}
               							  };
	int option_index = 0;
	
	if(argc < 2)
		printHelpAndExit(stderr, EXIT_FAILURE);

	n_days = atoi(argv[1]);
	
	do {
		opt = getopt_long(argc, argv, "hc", long_options, &option_index);

		switch (opt) {
		case 'h':
			printHelpAndExit(stderr, EXIT_SUCCESS);	
			break;
		case 'c':
			count_ = 1;
			break;
		case '?': 	
			fprintf(stderr,"Neznama volba -%c\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		//case ':':
		//	n_days = atoi(optarg);
		default:
			break;
		}
		
	} while(opt != -1);

	if(optind + 1 < argc ) {
		strncpy(file_path, argv[argc - 1], sizeof(file_path));
	} else {
		getPathFromChild();
	} 
}

void readDir(char* dir){
	DIR* directory;
	struct dirent* entry;
	int count = 0;
	size_t length;
	length = strlen(dir);
	if (dir[length - 1] != '/') {
    		dir[length] = '/';
    		dir[length + 1] = '\0';
    		++length;
  	}
	if((directory = opendir(dir)) == NULL){
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	struct stat st;
	char file[PATH_MAX + 1];
	strcpy(file, dir);
	time_t my_time;
	time(&my_time);
	time_t n_sec = n_days * 24 * 3600;
	while(( entry = readdir(directory)) != NULL) {
		if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
    			strncpy(file + length, entry->d_name, sizeof(file) - length);
    			lstat (file, &st);
			if(S_ISDIR (st.st_mode)){
				pid_t pid = fork(); //musi byt aby sme mohli rekurzivne volat(nemoze byt otvorenzch tolko dir jednym porcesom
				if(!pid){
					readDir(file);
					exit(EXIT_SUCCESS);
				} else {
					wait(NULL);
				}
			}
			if(S_ISREG (st.st_mode)){
				if((my_time - st.st_mtim.tv_sec) < n_sec){
					if(count_){
						count++;
					} else {
						fprintf(stderr, "%s\n", file);
					}
				}
			}
		}
	}
	if(count_)
		fprintf(stderr, "# of reg files in %s modified less than %i is %i\n", dir, n_days, count);
	if(closedir(directory) != 0 ) {
		perror("closedir");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char * argv[]){

	parseArgs(argc, argv);
//	while(1){
	readDir(file_path);
//		getPath();
//	}
	return EXIT_SUCCESS;
}
