#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//makro pre jednoduchsiu kontrolu navratovej hodnoty volania funkcie a vypis popisu chyby
#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

#define SEG_SIZE 0x6400

// makra, struktury, konstanty, ...
typedef enum {
    UNSET=0,
    SET=1
} OPTION;

typedef struct {
    OPTION h;
		OPTION o;
		char * fileArg;
} ARGS;

// buffer pre nacitanie vstupu od usera
char userInput[1024];

// Debug vypis
int debug=1;

/************************ deklaracie fcii *****************************/

// vypise help
void printHelp(FILE* stream);
// vypis ehelp a ukonci program
void printHelpAndExit(FILE* stream, int exitCode);
// nacita vstupne prepinace/argumenty
void parseArguments(int argc, char * argv[], ARGS * args);
// zvaliduje vstupne argumenty
void validateArguments(ARGS * args);
// Zobrazi vsetky regularne nespustitelne subory v aktualnom priecinku a podpriecinkoch
void showRegularFiles();
// Detske procesy na citanie zo suboru a nasledne zapisovanie do druheho suboru zadaneho podla argumentu
int child1_function(int in_fd, int out_fd, int seg_id);
int child2_function(int in_fd, int out_fd, int seg_id);
// Rodicovsky proces na koordinovanie zapisu/citania oboch detskych procesov
int parent(pid_t child1, pid_t child2, int seg_id_child1, int seg_id_child2);
// Vrati dlzku riadku - potrebne pre read aby vedel kolko ma precitat
int get_line_length(int fd);

// Funkcie na pracu s pamatou
void incrementShm(int seg_id);
void resetShm(int seg_id);
int getShmInt(int seg_id);

// MAIN
int main(int argc, char * argv[]) {
	int in_fd, out_fd, seg_id_child1, seg_id_child2;
	ARGS args;
	pid_t child1, child2;

	sigset_t sigsetMask;
	sigemptyset( & sigsetMask);
	sigaddset( & sigsetMask, SIGUSR1);
	sigprocmask( SIG_BLOCK, & sigsetMask, NULL);

	// Pamat pre child1
	CHECK( (seg_id_child1 = shmget(IPC_PRIVATE, SEG_SIZE, IPC_CREAT | 0600)) != -1 );
	resetShm(seg_id_child1);
	// Pamat pre child2
	CHECK( (seg_id_child2 = shmget(IPC_PRIVATE, SEG_SIZE, IPC_CREAT | 0600)) != -1 );
	resetShm(seg_id_child2);

	parseArguments(argc, argv, &args);
	validateArguments(&args);

	fprintf(stderr,"\n ***** Rekurzivny vypis regularnych nespustitelnych suborov ***** \n");
	showRegularFiles("./");
	fprintf(stderr,"\n\n ***** Teraz si vyber jeden z tychto suborov ***** \n");
  scanf("%199s", userInput);

	// Otvorenie input suboru, ktory bude procesmi citany
	CHECK( (in_fd = open(userInput, O_RDONLY)) != -1);
	// Otvorenie output suboru, do ktoreho budu procesy zapisovat
	CHECK( (out_fd = open(args.fileArg, O_WRONLY | O_CREAT, 0755 )) != -1);

	// Vytvorenie dvoch detskych procesov
	CHECK( (child1=fork()) != -1);
	if(child1==0) {
		child1_function(in_fd,out_fd,seg_id_child1);
	} else {
		CHECK( (child2=fork()) != -1);
		if (child2==0) {
			child2_function(in_fd,out_fd,seg_id_child2);
		} else {
			parent(child1,child2,seg_id_child1,seg_id_child2);
    }
  }
	return EXIT_SUCCESS;
}


/************************ definicie fcii *****************************/

int get_line_length(int fd) {
	char c;
	int charIdx = 0;
	while( c != '\n' ) {
		if ( read(fd, &c, 1) == 0) break;
		charIdx++;
	}
	return charIdx;
}

int child1_function(int in_fd, int out_fd, int seg_id) {
	int receivedSignal;
	const int dataMaxLength = 500;
	char data[dataMaxLength+1];
	off_t start_of_line;
	int dataLength=-1;
	char identify_string[] = "[Child1 -> ";
	int pid = getpid();

	// Vytvorenie id stringu, ktory bude pouzivat proces pri zapise do output - podla zadania
	char buff_pid[256];
	sprintf(buff_pid, "%d", pid);
	int buff_len=strlen(buff_pid);
	buff_pid[buff_len] = ']';
	buff_pid[buff_len+1] = ' ';
	buff_pid[buff_len+2] = '\0';

	sigset_t sigsetMask;
	sigemptyset( & sigsetMask);
	sigaddset( & sigsetMask, SIGUSR1);

	while(dataLength!=0)
	{
		// Proces caka na signal od rodica
		if(debug) fprintf(stderr,"\n\n CHILD1: Cakam na signal... \n");
		sigwait(&sigsetMask, &receivedSignal);
		if(debug) fprintf(stderr,"\n\n CHILD1: Prijal som signal... \n");
		// lseek potrebny na zapamatanie si offset suboru (kde sa nachadzam)
		start_of_line=lseek(in_fd,0,SEEK_CUR);
		dataLength=get_line_length(in_fd);
		// Po zisteni velkosti riadku sa offset nastavi na povodnu hodnotu
		lseek(in_fd,start_of_line,SEEK_SET);

		if(dataLength!=0) {
			// Nacitanie z input suboru
			read(in_fd, data, dataLength);
			// Zapis do output suboru s id stringom
			write(out_fd, identify_string, sizeof(identify_string));
			write(out_fd, buff_pid, strlen(buff_pid));
			write(out_fd, data, dataLength);
		}
		// Odosolanie signalu rodicovskemu procesu - da vediet ze precital a zapisal riadok
		if(debug) fprintf(stderr,"\n\n CHILD1: Odosielam signal parentovi... \n");
		kill(getppid(),SIGUSR1);
		if(dataLength==0) {
			// V pripade, ze proces uz nacital cely subor, nastav zdielanu pamat a posli signal parentovi
			if(debug) fprintf(stderr,"\n\n ***** CHILD1: dataLegnth END - nastavujem pamat na 1 ***** \n");
			incrementShm(seg_id);
    	kill(getppid(),SIGUSR1);
			return EXIT_SUCCESS;
		}
	}
	if(debug) fprintf(stderr,"\n\n ***** CHILD1: END ***** \n");
	return EXIT_SUCCESS;
}

// Cela funkcia obdobna ako child1_function
int child2_function(int in_fd, int out_fd, int seg_id) {
	int receivedSignal;
	const int dataMaxLength = 500;
  char data[dataMaxLength+1];
	int dataLength=-1;
	off_t start_of_line;
	char identify_string[] = "[Child2 -> ";
	int pid = getpid();

  char buff_pid[256];
  sprintf(buff_pid, "%d", pid);
	int buff_len=strlen(buff_pid);
	buff_pid[buff_len] = ']';
	buff_pid[buff_len+1] = ' ';
	buff_pid[buff_len+2] = '\0';

	sigset_t sigsetMask;
  sigemptyset( & sigsetMask);
  sigaddset( & sigsetMask, SIGUSR1);

	while(dataLength!=0)
  {
		if(debug) fprintf(stderr,"\n\n CHILD2: Cakam na signal... \n");
    sigwait(&sigsetMask, &receivedSignal);
		if(debug) fprintf(stderr,"\n\n CHILD2: Prijal som signal... \n");
		start_of_line=lseek(in_fd,0,SEEK_CUR);
    dataLength=get_line_length(in_fd);
		lseek(in_fd,start_of_line,SEEK_SET);
		if(dataLength!=0) {
    		read(in_fd, data, dataLength);
				write(out_fd, identify_string, sizeof(identify_string));
				write(out_fd, buff_pid, strlen(buff_pid));
				write(out_fd, data, dataLength);
		}
		if(debug) fprintf(stderr,"\n\n CHILD2: Odosielam signal parentovi... \n");
    kill(getppid(),SIGUSR1);
		if(dataLength==0) {
			if(debug) fprintf(stderr,"\n\n ***** CHILD2: dataLegnth END - nastavujem pamat na 1 ***** \n");
			incrementShm(seg_id);
    	kill(getppid(),SIGUSR1);
			return EXIT_SUCCESS;
		}
  }
	if(debug) fprintf(stderr,"\n\n ***** CHILD2: END ***** \n");
	return EXIT_SUCCESS;
}

int parent(pid_t child1, pid_t child2, int seg_id_child1, int seg_id_child2) {
	int receivedSignal;
	pid_t status_child1=0, status_child2=0;

	sigset_t sigsetMask;
  sigemptyset( & sigsetMask);
  sigaddset( & sigsetMask, SIGUSR1);

	// Kontrola ci uz jeden z procesov neprecital cely subor (nenastavil svoju zdielanu pamat na 1)
	while((status_child1=getShmInt(seg_id_child1)) == 0 && (status_child2=getShmInt(seg_id_child2)) == 0)
	{
		if((status_child1=getShmInt(seg_id_child1)) == 0 && (status_child2=getShmInt(seg_id_child2)) == 0) {
			// Posli signal dietatu a cakaj kym dieta nacita/zapise
			if(debug) fprintf(stderr,"\n\n PARENT: Odosielam signal CHILD1... \n");
			kill(child1, SIGUSR1);
			if(debug) fprintf(stderr,"\n\n PARENT: Cakam na signal CHILD1... \n");
			sigwait(&sigsetMask, &receivedSignal);
			if(debug) fprintf(stderr,"\n\n PARENT: Prijal som signal od CHILD1... \n");

		}
		// Obdoba predosleho ifu pre druhe dieta
		if((status_child1=getShmInt(seg_id_child1)) == 0 && (status_child2=getShmInt(seg_id_child2)) == 0) {
			if(debug) fprintf(stderr,"\n\n PARENT: Odosielam signal CHILD2... \n");
			kill(child2, SIGUSR1);
			if(debug) fprintf(stderr,"\n\n PARENT: Cakam na signal CHILD2... \n");
			sigwait(&sigsetMask, &receivedSignal);
			if(debug) fprintf(stderr,"\n\n PARENT: Prijal som signal od CHILD2... \n");
		}
	}
	if(debug) fprintf(stderr,"\n\n ***** END PARENT ***** \n");
	return EXIT_SUCCESS;
}

void showRegularFiles(char * dir) {
  DIR* directory;
  struct dirent* entry;
  size_t length;
  length = strlen(dir);

	if (dir[length-1] != '/') {
		dir[length] = '/';
		dir[length+1] = '\0';
		++length;
	}

	CHECK( (directory = opendir(dir)) != NULL);

	struct stat st;
	char file[PATH_MAX+1];
	strcpy(file,dir);

	while(( entry = readdir(directory)) != NULL) {
		if( strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0) {
			strncpy(file + length, entry->d_name, sizeof(file) - length);
			lstat(file,&st);

			if(S_ISDIR (st.st_mode)) {
				pid_t pid = fork();
				if(!pid) {
					showRegularFiles(file);
					exit(EXIT_SUCCESS);
				} else {
					wait(NULL);
				}
			}
			// Ak je subor regularny a zaroven nema nastaveny execute bit na Ownerovi,Groupe,Others, tak ho vypis
			if(S_ISREG(st.st_mode)) {
				if(((st.st_mode & S_IXGRP) != S_IXGRP) && ((st.st_mode & S_IXOTH) != S_IXOTH) && ((st.st_mode & S_IXUSR) != S_IXUSR)) {
					fprintf(stderr, "\n %s", file);
				}
			}
		}
	}
}

// vypise help
void printHelp(FILE* stream) {
	fprintf(stream, "Usage: parametre [-h] -o <output>\n");
  fprintf(stream, "Prepinace:\n");
  fprintf(stream, " -h, --help   vypise help\n");
  fprintf(stream, " -o, --output vystupny subor\n");
}

// vypise help a ukonci program
void printHelpAndExit(FILE* stream, int exitCode) {
	printHelp(stream);
	exit(exitCode);
}

// nacita vstupne prepinace/argumenty
void parseArguments(int argc, char * argv[], ARGS * args) {
	int opt;
	struct option longOptions[] = {
		{"help"	, no_argument, NULL, 'h'},
		{"output"	, required_argument, NULL, 'o'},
		{NULL	, 0			 , NULL, 0  }
	};

	// inicializacia parametrov na default hodnoty
	args->h = UNSET;
	args->o = UNSET;

	// nacitaj parametre s prepinacom
	do {
		opt = getopt_long(argc, argv, ":ho:", longOptions, NULL);
		switch(opt) {
			case 'h':
				args->h = SET;
				printHelpAndExit(stderr,EXIT_FAILURE);
      	break;
			case 'o':
				args->o = SET;
				args->fileArg = optarg;
				break;
      case ':':
      	fprintf(stderr, "\n[CHYBA] povinny argument prepinaca: %c\n\n", optopt);
				printHelpAndExit(stderr,EXIT_FAILURE);
      	break;
      case '?':
      	fprintf(stderr, "\n[CHYBA] Neznamy prepinac: %c\n\n", optopt);
				printHelpAndExit(stderr,EXIT_FAILURE);
      	break;
     	}
    } while (opt != -1);
}

// zvaliduje vstupne argumenty
void validateArguments(ARGS * args) {
	if(args->o == UNSET) {
		fprintf(stderr, "\n[CHYBA] Nie je zadany povinny prepinac -o\n\n");
		printHelpAndExit(stderr,EXIT_FAILURE);
	}
}

// PRACA S PAMATOU
void incrementShm(int seg_id) {
    char *shared_mem = (char *)shmat(seg_id, 0,0);
    int count = atoi(shared_mem) + 1;
    sprintf(shared_mem,"%d",count);
    if(shmdt(shared_mem) == -1) {
        perror("shmdt:");
        exit(EXIT_FAILURE);
    }

}

void resetShm(int seg_id) {
    char *shared_mem = (char *)shmat(seg_id, 0,0);
    sprintf(shared_mem,"%d",0);
    if(shmdt(shared_mem) == -1) {
        perror("shmdt:");
        exit(EXIT_FAILURE);
    }
}

int getShmInt(int seg_id) {
    char *shared_mem = (char *)shmat(seg_id, 0,0);
    int value = atoi(shared_mem);
    if(shmdt(shared_mem) == -1) {
        perror("shmdt:");
        exit(EXIT_FAILURE);
    }

    return value;
}