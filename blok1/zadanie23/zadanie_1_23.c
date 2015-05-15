/*
Napiste program, ktory v zadanej adresarovej strukture najde vsetky regularne nespustitelne subory a poslednych <pocet> riadkov
kazdeho z nich zapise na standardny vystup. Ak nie je pouzity prepinac "-c", zapise sa iba posledny riadok. Adresarova struktura je
specifikovana pomocou pociatocneho adresara <pociatocny_priecinok>. Prehladavanie adresarovej struktury robte pomocou API
funkcii jadra systemu. Vystup zabezpecia dva procesy (rodic a potomok) podla ukazky. Rodic zapisuje uplnu cestu spracovavaneho
suboru, potomok pozadovane riadky.
Volanie programu: zadanie.elf [-c <pocet>] <pociatocny_priecinok>
Ukazka vystupu pre argument "-c 2": <cesta_suboru_1>
<predposledny riadok>
<posledny riadok>
<cesta_suboru_2>
<predposledny_riadok>
<posledny_riadok>
...
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <limits.h>
#include <stdbool.h>


#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }


typedef enum{
	UNSET = 0,
	SET = 1
} OPTION;

typedef struct {
	char *startDir;
	OPTION c;
	int lineCount;
	OPTION h;
} ARGS;

typedef struct {
	char *dir;
	int count;
} DIRINFO;

void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage: parametre <pociatocny_priecinok> [-r | --reverse ]\n");
	fprintf(stream, "Program vypise zoznam adresarov zadaneho argumentom <pociatocny_priecinok>\nzoradenych vzostupne, podla poctu vsetkych vnorenych regularnych suborov.\nPrepinacom [-r | --reverse ] menime vypis na zoradeny zostupne\nc");
	exit(exitCode);
}

/** 
* @brief Vynuluje argumenty
* 
* @param args
*/
void initArguments(ARGS *args) {
	args->c = UNSET;
	args->h = UNSET;
	args->lineCount = 1;
	args->startDir = NULL;
}

void parseArguments(int argc,char *argv[], ARGS *args) {
	initArguments(args);

	struct option long_opts[] = {
		{"help",0,NULL,'h'},
		{NULL,0,NULL,0}
	};
	int opt;
	int opt_index = 0;
	char *arg;
	do {
		opt = getopt_long(argc,argv,":c:h",long_opts, &opt_index);

		switch(opt) {
			case 'c':
				args->c = SET;
				args->lineCount = atoi(optarg);
				break;
			case 'h':
			    args->h = SET;
			    break;
			case ':':
				fprintf(stderr, "Heej chyba ti parameter\n");
				printHelpAndExit(stderr, EXIT_FAILURE);
			case '?':
				fprintf(stderr,"Neznama volba -%c\n",optopt);
				printHelpAndExit(stderr, EXIT_FAILURE);
			default:
				break;
		}
	} while(opt != -1);
	
	// nacitaj ostatne prepinace
	while (optind < argc) {
		arg = argv[optind];
		if (args->startDir == NULL && arg != NULL) {
			args->startDir = arg;
		} else {
			printf("Ignorovany argument bez prepicana: %s\n", arg);
		}
		
		optind++;
	}

}


/** 
* @brief Nacitame <pociatocny_priecinok>
* ak je to regularny subor / ktory nie je spustitelny  !( st.st_mode & S_IXUSR)
* tak nacitame jeho absolute path a v childovi vypiseme cez tail menoSuboru -n pocetRiadkov
* @param startDir
* @param maskStr
*/
void listDir(char *startDir, int lastLines) {
	
	
	DIR *dir;
	struct dirent *entry;
	size_t dirLength = strlen(startDir);
	char dirName[PATH_MAX];
	strncpy(dirName, startDir, dirLength);
	


//ak neni v ceste priecinka na konci "/" pridame ho
	if(dirName[dirLength - 1] != '/') {
		dirName[dirLength] = '/';
		dirName[dirLength + 1] = '\0';
		dirLength++;
	}
	CHECK((dir = opendir(dirName)) != NULL);

	struct stat st;
	char file[PATH_MAX + 1];
	strcpy(file, dirName);

	
	while( (entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			strncpy(file + dirLength, entry->d_name, sizeof(file) - dirLength);
			
			CHECK(lstat(file, &st) != -1);


						
			if (S_ISREG(st.st_mode) && !(st.st_mode & S_IXUSR)) {
				char *absPath = realpath(file, NULL);
				char lastLinesStr[4];
				sprintf(lastLinesStr,"%d",lastLines);
				printf("%s\n", absPath);
				
				pid_t pid = fork();
				switch (pid) {
					case -1:
						perror("fork");
						exit(EXIT_FAILURE);
					case 0:
						execlp("tail", "tail",absPath,"-n",lastLinesStr,NULL);
						perror("execv");	
						exit(EXIT_FAILURE);
					default:
						wait(NULL);
						break;
						
				}
			}
			
		}
	}
	
	closedir(dir);
	
}


void executeChoice(ARGS *args) {
	listDir(args->startDir, args->lineCount);
}

void validateARGS(ARGS *args) {
	if(args->startDir == NULL) {
		printf("Nebolo by zle zadat <pociatocny_precinok>\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
}
void printARGS(ARGS *args) {
	printf("c: %30s\n",args->c ? "SET" : "UNSET");
	printf("linesCount: %30d\n",args->lineCount);
	printf("help: %30s\n",args->h ? "SET" : "UNSET");	
	printf("dir: %30s\n",args->startDir);
}
int main(int argc, char *argv[]) {
	ARGS args;
	parseArguments(argc,argv,&args);
	validateARGS(&args);
	if( args.h ) {
	    printHelpAndExit(stderr, EXIT_SUCCESS);
    }
	executeChoice(&args);
	return 0;
}
