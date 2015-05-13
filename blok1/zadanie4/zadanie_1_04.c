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

#define FIFO_NAME "fifo-sp04"

char file_path[PATH_MAX + 1];
int  exist_ = 0;
int  not_exist_ = 0;

int child(){
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
	return 1;
}

void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage: parametre [-h] [-e | --exist | -n | --not_exist] [<dir>]\n");
	fprintf(stream, "Program vypise vsetky symbolicke linky v adresarovej strukture. Ak je definovany prepinac, zobrazi sa len (ne)existujucimi subormi\n");
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
                   						  {"exist", 0, NULL, 'e'},
                   						  {"not_exist", 0, NULL, 'n'},
                   						  {0, 0, 0, 0}
               							  };
	int option_index = 0;

	do {
		opt = getopt_long(argc, argv, "hen", long_options, &option_index);

		switch (opt) {
		case 'h':
			printHelpAndExit(stderr, EXIT_SUCCESS);	
			break;
		case 'e':
			exist_ = 1;
			break;
		case 'n':
			not_exist_ = 1;
			break;
		case '?': 	
			fprintf(stderr,"Neznama volba -%c\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		default:
			break;
		}
		
	} while(opt != -1);

	if(optind < argc ) {
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

	while(( entry = readdir(directory)) != NULL) {
		if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
    			strncpy(file + length, entry->d_name, sizeof(file) - length);
    			lstat (file, &st);
			if(S_ISDIR(st.st_mode)){
				pid_t pid = fork(); //musi byt aby sme mohli rekurzivne volat(nemoze byt otvorenzch tolko dir jednym porcesom
				if(!pid){
					readDir(file);
					exit(EXIT_SUCCESS);
				} else {
					wait(NULL);
				}
			}
			if(S_ISLNK(st.st_mode)){
				if(!(exist_ || not_exist_))
					fprintf(stderr, "%s\n", file);
				if(exist_ && access(file, F_OK) == 0)
					count++;
				if(not_exist_ && access(file, F_OK) == -1 && errno == ENOENT)
					count++; 
			}
		}
	}	
	if(exist_ || not_exist_)
		fprintf(stderr,"# of symlinks is %i\n", count);
	if(closedir(directory) != 0 ) {
		perror("closedir");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char * argv[]){

	parseArgs(argc, argv);
	//while(1){
		readDir(file_path);
	//	getPath();
	//}
	return EXIT_SUCCESS;
}
