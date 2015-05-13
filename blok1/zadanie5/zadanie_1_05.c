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

//makro pre jednoduchsiu kontrolu navratovej hodnoty volania funkcie a vypis popisu chyby
#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

// identifikator komunikacneho fifa
#define FIFO_NAME "fifo_pipe"	

// maximalna dlzka spravy
#define MSG_MAX_LENGTH 200

// makra, struktury, konstanty, ...
typedef enum {
    UNSET=0,
    SET=1
} OPTION;

typedef struct {
    OPTION h;
    OPTION m;
    char * maskWord;
    OPTION f;
    char * filePath;
    OPTION c;
char * pathArg;
} ARGS;

typedef struct {
	char msg[MSG_MAX_LENGTH];
	int msgId;
} PIPE_MSG;

// buffer pre nacitanie vstupu od usera
char userInput[MSG_MAX_LENGTH];	

// flag ci bol prijaty signal SIGCHLD, 1 - ano, 0 -nie
sig_atomic_t sigChldFlag;	

/************************ deklaracie fcii *****************************/

// vypise nacitane argumenty
void printArgs(ARGS * args);
// vypise help
void printHelp(FILE* stream);
// vypis ehelp a ukonci program
void printHelpAndExit(FILE* stream, int exitCode);
// nacita vstupne prepinace/argumenty
void parseArguments(int argc, char * argv[], ARGS * args);
// zvaliduje vstupne argumenty
void validateArguments(ARGS * args);
// nastavi vstupne argumenty na default hodnoty
void resetArguments(ARGS * args);


/* zdruzuje komunikacnu cast kodu - nacitanie vstupu od usera v child procese, cakanie na signal v 
parent procese, prijatie spravy, zapisanie do args */
void processCommun(ARGS * args);
// handler pri signal SIGCHLD
void sigChldHandler();
// nacita vstup od usera a zapise ho do fifa
void sendUserInput_child(ARGS * args);
// rekurzivne do hlbky vypise vsetky regularne vyrazy v aktualnom priecinku
void displayDirContentRec(char * directory);
// caka na signal od childa(SIGCHILD), precita spravu z fifa a naplni strukturu args
void receiveUserInput_parent(ARGS * args); 
// nacita vstup do globalneho pola charov od uzivatela a vrati ho ako ukazatel
char * getUserInput();

// vykona logiku programu, t.j. napr. rekurzivne nacitanie adresarovej strukturi, zistenie nejakych info, ...
void processProgramLogic(ARGS * args);


int main(int argc, char * argv[]) {
	ARGS args;
	resetArguments(&args);
	parseArguments(argc, argv, &args);
	validateArguments(&args);

	while(1) {
		processProgramLogic(&args);
		processCommun(&args);
	}
	return EXIT_SUCCESS;
}


/************************ definicie fcii *****************************/

/* zdruzuje komunikacnu cast kodu - nacitanie vstupu od usera v child procese, cakanie na signal v 
parent procese, prijatie spravy, zapisanie do args */
void processCommun(ARGS * args) {
	// ak je nastavena cesta k suboru, returnuj
	if (args->f == SET) return;
	
	/* handler pre SICHCHLD(ukoncenie detskeho procesu), tento signal podmienuje 
	pokracovanie cinnosti rodicovseko procesu, obdobne by sa naviazali aj ine signaly */
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &sigChldHandler;

	// nastav handler pre SIGCHLD
	sigaction(SIGCHLD, &sa, NULL);

	pid_t pid;
	CHECK( (pid = fork()) != -1 );
	switch(pid) {
		case 0:	// detsky proces
			sendUserInput_child(args);
			exit(EXIT_SUCCESS);
		default:
			receiveUserInput_parent(args);
			while(sigChldFlag == 0) continue;	// cakaj na prijatie signalu od childa
			// signal bol prijaty --> zresetuj flag a pokracuj
			printf("Komunikacia skoncila - hlavny beh programu moze pokracovat\n");
			sigChldFlag = 0;
			return;
	}

}

// handler pri signal SIGCHLD
void sigChldHandler() {
	sigChldFlag = 1;
}

// nacita vstup od usera a posle ho cez pipu druhemu procesu
void sendUserInput_child(ARGS * args) {
	int fifo;

	// vytvor komunikacny kanal - pipu
	umask(0);
	CHECK( (mkfifo(FIFO_NAME, 0660) != -1) || (errno == EEXIST) );	// vytvori fifo
	CHECK( (fifo = open(FIFO_NAME, O_WRONLY)) != -1 );	// otvori fifo - ziska deskriptor
	CHECK( unlink(FIFO_NAME) != -1 );	// zmaze fifo

	// zisti aktulny adresar
	char currDir[PATH_MAX];
	CHECK( getcwd(currDir, PATH_MAX) != NULL );
	
	// vypis rekurzivne obsah aktualneho adresara - user si ma vybrat nejaky subor z tohto vypisu
	printf("\nZoznam suborov: \n");
	displayDirContentRec(currDir);

	printf("Staci pisat bez: %s\n", currDir);
	// nacitaj vstup od usera
	char *input = getUserInput();
	PIPE_MSG locMsg;	// sprava
	locMsg.msgId = 0;	// keby je treba da sa dorobit counter pri identifikaciu poradia sprav
	strncpy(locMsg.msg, input, sizeof(locMsg.msg));

	// posli spravu
	CHECK( write(fifo, &locMsg, sizeof(locMsg)) != -1);

	// zatvori deskriptor
	CHECK( close(fifo) != -1 );
	// tu by sa poslal signal parentovi, keby mame pouzit iny ako SIGCHLD
}

// rekurzivne do hlbky vypise vsetky regularne vyrazy v danom priecinku
void displayDirContentRec(char * directory) {
	char path[PATH_MAX];	// pri rekurziach pracuj vzdy s lokalnou kopiou
	DIR * dirStream;
	CHECK( (dirStream = opendir(directory)) != NULL);	// otvori adresar

	struct dirent * dirEntry;	// struktura pre 1 polozku v adresari
	struct stat info;	// struktura pre info o subore
	char * fileName;

	while ( (dirEntry = readdir(dirStream)) != NULL) {
		fileName = dirEntry->d_name;
		if ( strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0 ) continue;	// ignoruj adresare . a ..

		strncpy(path, directory, strlen(directory));
		path[strlen(directory)] = '\0';
		strcat(path, "/");
		strcat(path, fileName);

		CHECK( lstat(path, &info) != -1 ) // nacita info o subore
		if ( S_ISREG(info.st_mode) == 1) {	// regularny subor
			printf("\tfile: %s\n", path);
		}
		if ( S_ISDIR(info.st_mode) == 1) {	// adresar
			pid_t pid;		
			CHECK( (pid = fork()) != -1 );	// v jednom procese nemoze byt vela adresaro otvorenych naraz
			switch(pid) {
				case 0:
					displayDirContentRec(path);			
					exit(EXIT_SUCCESS);
				default:
					wait(NULL);
			}
		}
	}
}

// caka na signal od childa(SIGCHILD), precita spravu z fifa a naplni strukturu args
void receiveUserInput_parent(ARGS * args) {
	int fifo;

	// vytvor komunikacny kanal - pipu
	umask(0);
	CHECK( (mkfifo(FIFO_NAME, 0660) != -1) || (errno == EEXIST) );	// vytvori fifo
	CHECK( (fifo = open(FIFO_NAME, O_RDONLY)) != -1 );	// otvori fifo - ziska deskriptor
	//CHECK( unlink(FIFO_NAME) != -1 );	// zmaze fifo

	PIPE_MSG locMsg;	// sprava
	
	// prijmi spravu
	CHECK ( read(fifo, &locMsg, sizeof(locMsg)) == sizeof(locMsg) );

	// vypis prijatu spravu
	printf("Prijata sprava: %s\n", locMsg.msg);
	
	// zatvori deskriptor
	CHECK( close(fifo) != -1 );
	
	// ak si prijal k-koniec, skonci
	if (strcmp(locMsg.msg, "k") == 0) exit(EXIT_SUCCESS);

	// inak napln strukturu args
	args->f = SET;
	args->filePath = locMsg.msg;
} 

// nacita vstup do globalneho pola charov od uzivatela a vrati ho ako ukazatel
char * getUserInput() {
	printf("Zadajte cestu k suboru:\n\"k\" pre koniec\n");
	scanf("%199s", userInput);
	return userInput;
}

// vykona logiku programu, t.j. napr. rekurzivne nacitanie adresarovej strukturi, zistenie nejakych info, ...
void processProgramLogic(ARGS * args) {
	// ak nieje nastaveny file, returnuj
	if (args->f == UNSET) return;	

	printf("\nSlovo %s sa vyskytuj v nasledujucich riadkov suboru %s\n", args->maskWord, args->filePath);

	// skontroluj ci zadany subor existuje a je to regularny file
	//struct stat info;

	int fileDesc;
	CHECK( (fileDesc = open(args->filePath, O_RDONLY)) != 1 );	// otvor subor

	// ak > lineSize by sa malo realokovat a riadok by mohol mat hocikolko znakov, ale nasrac
	char line[500];
	memset(line, 0, 500);
	char c;
	int charIdx = 0;
	int maskWordOccurencies = 0;

	while( c != EOF && charIdx<500) {
		if ( read(fileDesc, &c, 1) == 0) break;		// 0 indikuje EOF
		if (c == '\n') {	// novy riadok, skontroluj vyskyt slova			
			line[charIdx+1] = '\0';
			charIdx = 0;
		
			if (strstr(line, args->maskWord) != NULL) {	// nasli sme substring v riadku
				printf("line: %s\n", line);
				maskWordOccurencies++;
			}
		}
		line[charIdx] = c;
		charIdx++;
	}	

	if (args->c == SET) {
		printf("\nPocet riadkov v ktorom sa vyskytlo slovo \"%s\": %d\n", args->maskWord, maskWordOccurencies);
	}

	// po dokonceni resetuj filePath
	args->f = UNSET;
	args->filePath = "default";
}

// vypise nacitane argumenty
void printArgs(ARGS * args) {
	printf("Ncitane parametre: \n");
	printf("\thelp: %d\n", args->h);
	printf("\tmask: %d\n", args->m);
	printf("\tmaskWord: %s\n", args->maskWord);
	printf("\tfile: %d\n", args->f);
	printf("\tfilePath: %s\n", args->filePath);
	printf("\tcount: %d\n", args->c);
}

// vypise help
void printHelp(FILE* stream) {
	fprintf(stream, "Usage: parametre [-h|--help] -m|--mask<mask> [-f|--file][file][-c|--count]\n");
    fprintf(stream, "Prepinace:\n");
    fprintf(stream, " -h, --help  vypise help\n");
    fprintf(stream, " -f, --file  prehladany subor\n");
    fprintf(stream, " -h, --mask  hladany vyraz\n");
    fprintf(stream, " -c, --count  vypise pocet vyskytov slova\n");
}

// vypis ehelp a ukonci program
void printHelpAndExit(FILE* stream, int exitCode) {
	 printHelp(stream);
    exit(exitCode);
}

// nacita vstupne prepinace/argumenty
void parseArguments(int argc, char * argv[], ARGS * args) {
	int opt;
    char * arg;
    struct option longOptions[] = {
        {"help"	, no_argument, NULL, 'h'},
        {"file"	, no_argument, NULL, 'f'},
        {"mask"	, no_argument, NULL, 'm'},
        {"count", no_argument, NULL, 'c'},
        {NULL	, 0			 , NULL, 0  }
    };
    
    // nacitaj parametre s prepinacom
    do {
        opt = getopt_long(argc, argv, ":hf:m:c", longOptions, NULL);
        switch(opt) {
            case 'h':
				args->h = SET;
				printHelp(stdout);
                break;
            case 'f':
				args->f = SET;
				args->filePath = optarg;
                break;
			case 'm':
				args->m = SET;
				args->maskWord = optarg;
                break;
			case 'c':
				args->c = SET;
                break;
			case ':':
                fprintf(stderr, "Chyba povinny argument prepinaca: %c\n", optopt);
                printHelpAndExit(stderr, EXIT_FAILURE);
            case '?':
                fprintf(stderr, "Neznamy prepinac: %c\n", optopt);
                printHelpAndExit(stderr, EXIT_FAILURE);
        }
    } while (opt != -1);

	/*
    // nacitaj ostatne prepinace
    while (optind < argc) {
        arg = argv[optind];
        if (args->path == UNSET && arg != NULL) {
            args->path = SET;
            args->pathArg = arg;
        } else {
            printf("Ignorovany argument bez prepicana: %s\n", arg);
        }
        optind++;
    }
    */
}

// zvaliduje vstupne argumenty
void validateArguments(ARGS * args) {
	if (args->m == UNSET) { 
		fprintf(stderr, "Nebol zadany povinny prepinac -m\n");		
		printHelpAndExit(stderr, EXIT_FAILURE);
        }
    printArgs(args);
}

// nastavi vstupne argumenty na default hodnoty
void resetArguments(ARGS * args) {
	args->h = UNSET;
	args->m = UNSET;
	args->maskWord = "default";
	args->f = UNSET;
	args->filePath = "default";
	args->c = UNSET;
}


