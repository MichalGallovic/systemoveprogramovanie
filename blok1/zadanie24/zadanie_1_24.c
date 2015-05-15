/*
POPIS 
Napiste program, ktory v zadanej adresarovej strukture najde vsetky spustitelne subory a na standardny vystup vypise zoznam tych
suborov, ktore obsahuju retazec podla zadanej masky <mask> prepinaca "-m" (emulacie prikazu grep). Zoznam na kazdom riadku
obsahuje absolutnu cestu suboru, ktory vyhovuje podmienke. Adresarova struktura je specifikovana pomocou pociatocneho
adresara <pociatocny_priecinok>. Prehladavanie adresarovej struktury robte v procese rodica pomocou API funkcii jadra systemu.
Overenie, ci dany spustitelny subor obsahuje retazec podla zadanej masky, robte v procese potomka. Zvazte moznost urychlenia
spracovania adresarovej struktury pomocou asynchronnej cinnosti potomkov vzhladom na rodica (tip: je ale vhodne, ak je
spracovanie vystupu synchronizovane; preco?)
Volanie programu: zadanie.elf -m <mask> <pociatocny_priecinok>
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

#define FIFO_PATH "/tmp/fifo_zad_1_25_ref"

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

#define SEG_SIZE 0x6400

typedef enum{
	UNSET = 0,
	SET = 1
} OPTION;

typedef struct {
	char *startDir;
	OPTION m;
	OPTION h;
	char *maskStr;
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
	args->m = UNSET;
	args->h = UNSET;
	args->maskStr = NULL;
	args->startDir = NULL;
}

void parseArguments(int argc,char *argv[], ARGS *args) {
	initArguments(args);

	struct option long_opts[] = {
		{"mask",1,NULL,'m'},
		{"help",0,NULL,'h'},
		{NULL,0,NULL,0}
	};
	int opt;
	int opt_index = 0;
	char *arg;
	do {
		opt = getopt_long(argc,argv,":m:h",long_opts, &opt_index);

		switch(opt) {
			case 'm':
				args->m = SET;
				args->maskStr = optarg;
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
* @brief Vracia true/false ak subor obsahuje maskStr vo file
* 
* @param file
* @param maskStr
* 
* @return 
*/
bool fileContainsString(char *file, char *maskStr) {
	FILE *stream;
	long file_size;
	char *file_contents;
	
	stream = fopen(file, "r");
	fseek(stream, 0, SEEK_END);
	file_size = ftell(stream);
	rewind(stream);
	file_contents = malloc(file_size * sizeof(char));
	fread(file_contents, sizeof(char), file_size, stream);
	fclose(stream);	
	
	if(strstr(file_contents, maskStr) == NULL) {
		return false;
	}
	
	return true;
}


/** 
* @brief Nacitame <pociatocny_priecinok>
* ak je to regularny subor / ktory je spustitelny ( st.st_mode & S_IXUSR)
* tak ho v childovi otvorime a skusime cez strstr v nom najst string, zadany prepinacom -m
* ak je tak vratime parentovi SIGUSR1 ak nie SIGUSR2
* 
* @param startDir
* @param maskStr
*/
void listDir(char *startDir, char *maskStr) {
	
	
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


						
			if (S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)) {
				sigset_t set;
				int signum;
				sigemptyset(&set);
				sigaddset(&set, SIGUSR1);
				sigaddset(&set, SIGUSR2);
				sigprocmask(SIG_SETMASK, &set, NULL);
				pid_t pid = fork();
				switch (pid) {
					case -1:
						perror("fork");
						exit(EXIT_FAILURE);
					case 0:
						if(fileContainsString(file, maskStr)) {
							kill(getppid(), SIGUSR1);
						} else {
							kill(getppid(), SIGUSR2);
						}
						exit(EXIT_SUCCESS);
					default:
						sigwait(&set, &signum);
						if(signum == SIGUSR1) {
							printf("%s\n", realpath(file, NULL));
						}	
						break;
						
				}
			}
			
		}
	}
	
	closedir(dir);
	
}


void executeChoice(ARGS *args) {
	listDir(args->startDir, args->maskStr);
//	sortResults(dir_array, count, args);
//	printResults(dir_array, count, args);
}

void printARGS(ARGS *args) {
	printf("mask: %30s\n",args->m ? "SET" : "UNSET");
	printf("maskPath: %30s\n",args->maskStr);
	printf("help: %30s\n",args->h ? "SET" : "UNSET");	
	printf("dir: %30s\n",args->startDir);
}
void validateARGS(ARGS *args) {
	if(args->m == UNSET) {
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
	
	if(args->startDir == NULL) {
		printf("Nebolo by zle zadat <pociatocny_precinok>\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
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
