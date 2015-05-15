/*
POPIS 
Zobrazte do standardneho vystupu zoznam vsetkych podadresarov v adresarovej strukture usporiadanych podla poctu suborov,
ktore obsahuju (tip: skuste na usporiadanie vyuzit napriklad funkciu "sort" zo standardnej kniznice jazyka C). Do poctu sa pocitaju aj
subory z jeho podpriecinkov. Adresarova struktura je specifikovana pomocou pociatocneho adresara <pociatocny_priecinok>. Tento
adresar je specifikovany povinnym prvym parametrom programu. V pripade, ze je definovany prepinac "-r" alebo "--reverse",
zobrazia sa adresare v opacnom poradi. Program nesmie pouzit volanie externeho programu. Program musi prehladavat pociatocny
adresar rekurzivne pomocou API funkcii jadra systemu. Spracovanie adresarovej struktury musi byt vykonane v procese rodica,
spocitanie poctu suborov v procese potomka. Zvazte moznost urychlenia spracovania adresarovej struktury pomocou asynchronnej
cinnosti potomkov vzhladom na rodica.
Volanie programu: zadanie.elf <pociatocny_priecinok> [-r | --reverse]
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

#define FIFO_PATH "/tmp/fifo_zad_1_25_ref"

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

#define SEG_SIZE 0x6400

typedef enum{
	UNSET = 0,
	SET = 1
} OPTION;

typedef struct {
	char *startDir;
	int h;
	int r;
} ARGS;

typedef struct {
	char *dir;
	int count;
} DIRINFO;

DIRINFO *dir_array;
int dir_arr_size = 20;
int count = 0;

pid_t master_parent_pid;


void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "\nUsage: parametre <pociatocny_priecinok> [-r | --reverse ]\n");
	fprintf(stream, "Program vypise zoznam adresarov zadaneho argumentom <pociatocny_priecinok> zoradenych vzostupne, podla poctu vsetkych vnorenych regularnych suborov. Prepinacom [-r | --reverse ] menime vypis na zoradeny zostupne");
	exit(exitCode);
}

/** 
* @brief Vynuluje argumenty
* 
* @param args
*/
void initArguments(ARGS *args) {
	args->r = UNSET;
	args->startDir = NULL;
}

void parseArguments(int argc,char *argv[], ARGS *args) {
	initArguments(args);

	struct option long_opts[] = {
		{"reverse",0,NULL,'r'},
		{"help",0,NULL,'h'},
		{NULL,0,NULL,0}
	};
	int opt;
	int opt_index = 0;
	char *arg;
	do {
		opt = getopt_long(argc,argv,"rh",long_opts, &opt_index);

		switch(opt) {
			case 'r':
				args->r = SET;
				break;
			case 'h':
			    args->h = SET;
			    break;
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
		if (args->startDir == UNSET && arg != NULL) {
			args->startDir = arg;
		} else {
			printf("Ignorovany argument bez prepicana: %s\n", arg);
		}
		optind++;
	}

}

/** 
* @brief Inkrementuje o 1 zdielanu pamat (int)
* 
* @param seg_id
*/
void incrementShm(int seg_id) {
	char *shared_mem = (char *)shmat(seg_id, 0,0);
	int count = atoi(shared_mem) + 1;
	sprintf(shared_mem,"%d",count);
	if(shmdt(shared_mem) == -1) {
		perror("shmdt:");
		exit(EXIT_FAILURE);
	}

}

/** 
* @brief Inicializuje zdielanu pamat na 0
* 
* @param seg_id
*/
void resetShm(int seg_id) {
	char *shared_mem = (char *)shmat(seg_id, 0,0);
	sprintf(shared_mem,"%d",0);
	if(shmdt(shared_mem) == -1) {
		perror("shmdt:");
		exit(EXIT_FAILURE);
	}
}

/** 
* @brief Vrati aktualnu hodnotu v zdielanej pamati
* 
* @param seg_id
* 
* @return int
*/
int getShmInt(int seg_id) {
	char *shared_mem = (char *)shmat(seg_id, 0,0);
	int value = atoi(shared_mem);
	if(shmdt(shared_mem) == -1) {
		perror("shmdt:");
		exit(EXIT_FAILURE);
	}
	
	return value;
}

/** 
* @brief Vytvori zdielanu pamat velkosti SEG_SIZE
* 
* @return (int) id zdielanej pamate 
*/
int initShm() {
	int seg_id = shmget(IPC_PRIVATE, SEG_SIZE, IPC_CREAT | 0600);
	if(seg_id == -1) {
		perror("shmget:");
		exit(EXIT_FAILURE);
	}
	resetShm(seg_id);
	
	return seg_id;
}

/** 
* @brief Hlavna funkcia, prehlada zadany priecinok
* sanitizne dirName ak neopsahuje trailing slash
* 
* V detoch spocitavame do zdielanej pamate pocet regularnych suborov
* v prvom parentovi - takze vzdy v jednom z adresarov, nachadzajucich sa
* v priecinku definovanom argumentom <pociatocny_priecinok>
*
* Pouzivame pole struktur - struktura DIRINFO obs, nazov adresara kt. reprezentuje a pocet jeho vnorenych reg. suborov 
* 
* @param args
*/
void listDir(char *startDir, int seg_id) {
	
	
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

	
	if(master_parent_pid == getpid()) {
		dir_array = (DIRINFO *) malloc(dir_arr_size*sizeof(DIRINFO));
	}
	
	while( (entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			strncpy(file + dirLength, entry->d_name, sizeof(file) - dirLength);
			
			CHECK(lstat(file, &st) != -1);


			pid_t pid;
			if(S_ISDIR(st.st_mode)) {
				pid = fork();
				if(pid == 0) {
					listDir(file, seg_id);
					exit(EXIT_SUCCESS);
				} else {
					wait(NULL);
					if(master_parent_pid == getpid()) {
						int file_count = getShmInt(seg_id);

						resetShm(seg_id);
						if(count > dir_arr_size) {
							dir_arr_size+=10;
							dir_array = realloc(dir_array, dir_arr_size);	
						}
						
						dir_array[count].dir = (char *)malloc(strlen(file) * sizeof(char));
						strncpy(dir_array[count].dir, file, strlen(file));
						dir_array[count].count = file_count;
						count++;
					}
				}
			}
			
			if (S_ISREG(st.st_mode)) {
				if(master_parent_pid != getpid()) {
					incrementShm(seg_id);
				}
			}
			
		}
	}
	
	if(master_parent_pid == getpid()) {
		
	}

	
	closedir(dir);
	
}

void sortResults(DIRINFO *dir_array, int size, ARGS *args) {
	int i = 0, j = 0;
	// basic bubble sort
	for (i = 0 ; i < size - 1; i++)
	{
		for (j = 0 ; j < size - i - 1; j++)
		{
			if(args->r) {
				if (dir_array[j].count < dir_array[j+1].count) /* For decreasing order use < */
				{
					DIRINFO swap = dir_array[j];
					dir_array[j]   = dir_array[j+1];
					dir_array[j+1] = swap;
				}				
			} else {
				if (dir_array[j].count > dir_array[j+1].count) /* For decreasing order use < */
				{
					DIRINFO swap = dir_array[j];
					dir_array[j]   = dir_array[j+1];
					dir_array[j+1] = swap;
				}	
			}

		}
	}
}

void printResults(DIRINFO *dir_array, int size, ARGS *args) {
	int i;
	printf("Vysledky zoradene %s :\n",args->r ? "zostupne" : "vzostupne");
	for(i = 0; i < size; i++) {
		printf("priecinok: %20s pocet: %20d\n", dir_array[i].dir, dir_array[i].count);
	}
}

void executeChoice(ARGS *args) {
	master_parent_pid = getpid();
	int seg_id = initShm();
	listDir(args->startDir, seg_id);
	sortResults(dir_array, count, args);
	printResults(dir_array, count, args);
}


int main(int argc, char *argv[]) {
	ARGS args;
	parseArguments(argc,argv,&args);
	if( args.h ) {
	    printHelpAndExit(stderr, EXIT_SUCCESS);
    }
	executeChoice(&args);
	return 0;
}
