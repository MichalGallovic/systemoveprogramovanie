#include "sem_wrapper.h"

// makra, struktury, konstanty, ...
typedef enum {
    UNSET=0,
    SET=1
} OPTION;

typedef struct {
    OPTION h;
    OPTION c;
    char * countArgStr;
    int  countArg;
} ARGS;

typedef struct {
	int befNum;
	int lastNum;
} PIPE_MSG;

int childTerminationFlag = 0;	// flag, ktory podmienuje infinite loop child procesu, nastavi sa po prijati signalu

#define FIFO_NAME "fifo-pipe_2" 

/*************************** deklaracie funkcii **********************************/

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
// zresetuje hodnoty argumentov
void resetArguments(ARGS * args);

// vytvori countArg pocet procesov, ktore budu synchronizovane mnozinou semaforou, vrati mnozinu ich pid
void processProgramLogic(pid_t * childrenPids, int childrenNum);
// pocka na posledny spravu od childa
void waitForLastChildMsg(int countNum);
// prisiel signal na ukoncenie, preposli tento signal vsetkym child procesom
void sendTermSignalToChildren(pid_t * childrenPids, int childrenNum);
// pocka na ukoncenie vsetkych child procesov
void waitForChildrenTermination(int childrenNum);
// synchonizovane pomocou semaforou zapisuje pid do pamate/na obrazovku v infinite loope, 
// ukonci sa po prijati signalu
void processChild(int childId, int childreNum);
// nastavi flag podmienujuci infitie loop child procesu na 1
void sigUsr1Handler();

/************************************ main ***************************************/

int main(int argc, char * argv[]){
	ARGS args;

	// spracuje vstupne prepinace/argumenty
	resetArguments(&args);
	parseArguments(argc, argv, &args);
	validateArguments(&args);
	// printArgs(&args);
	
	// inicializuje mnozinu semaforov
	initSem(args.countArg);

	printf("1\n");
	// vytvori countArg pocet procesov, ktore budu synchronizovane mnozinou semaforou, vrati mnozinu ich pid
	pid_t * childrenPids = (pid_t *) malloc(args.countArg * sizeof(pid_t));
	processProgramLogic(childrenPids, args.countArg);
	printf("2\n");

	// pocka na ukoncovaci signal - SIGINT
	waitForLastChildMsg(args.countArg);
	printf("3\n");

	// posledny child poslal spravu, preposli ukoncovaci signal vsetkym child procesom
	//sendTermSignalToChildren(childrenPids, args.countArg);

	// pocka na ukoncenie vsetkych child procesov
	waitForChildrenTermination(args.countArg);
	printf("Vsetky detske procesy ukoncene\n");

	// uprace pamat v ramci mnoziny semaforov
	deinitSem();

	return EXIT_SUCCESS;
}

/***************************** definicie funkcii *********************************/

// synchonizovane pomocou semaforou zapisuje pid do pamate/na obrazovku v infinite loope, 
// ukonci sa po prijati signalu
void processChild(int childId, int childrenNum) {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigUsr1Handler;
	
	// registruje handler pre SIGUSR1, ktory 
	CHECK( sigaction(SIGUSR1, &sa, NULL) != -1);

	printf("process child pred wait\n");
	waitSem( childId );	// caka na uvolnenie svojho semaforu
	printf("process child za wait\n");

	umask(0);
	CHECK( mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR) != -1 );

	// otvor fifo
	int fileDesc;
	CHECK( (fileDesc = open(FIFO_NAME, O_CREAT | O_RDONLY | O_WRONLY)) != -1 );

	// readFifo
	PIPE_MSG locMsg;
	CHECK( read(fileDesc, &locMsg, sizeof(locMsg)) != -1);

	int temp = locMsg.befNum;
	locMsg.befNum = locMsg.lastNum;
	locMsg.lastNum = temp + locMsg.lastNum;
	printf("childId: %d -> befNum: %d, lastNum; %d, result: %d\n", childId, temp, locMsg.befNum, locMsg.lastNum);
	
	// posli spravu
	CHECK( write(fileDesc, &locMsg, sizeof(locMsg)) != sizeof(locMsg) );

	// closeFifo
	CHECK( close(fileDesc) != -1 );

	int nextSem = childId +1;
	if ( nextSem > childrenNum ) nextSem = 0;	// posledny proces otvori semafor rodica
	enableSem( nextSem );	// uvolni dalsi semafor
}

// nastavi flag podmienujuci infitie loop child procesu na 1
void sigUsr1Handler() {
	printf("handler\n");
	childTerminationFlag = 1;	
}

// vytvori countArg pocet procesov, ktore budu synchronizovane mnozinou semaforou, vrati mnozinu ich pid
void processProgramLogic(pid_t * childrenPids, int childrenNum) {
	pid_t pid;

	/*
	int childId;
	// vytvori childrenNum detskych procesov
	for(childId = 2; childId < childrenNum; childId++) {
		CHECK( (pid = fork()) != -1 );
		switch(pid) {
			case 0:	// child
				processChild(childId, childrenNum);
				exit(EXIT_SUCCESS);
			default:	// parent
				childrenPids[childId] = pid;	// uloz pid child procesu
				break;
		}
	}
*/

	// posle udaje pre prvy child proces
	//umask(0);
	CHECK( (mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR) != -1) || (errno == EEXIST) );
	printf("2-1\n");

	// otvor fifo
	int fileDesc;
	printf(":(\n");
	CHECK( (fileDesc = open(FIFO_NAME, O_WRONLY)) != -1 );

	printf("2-2\n");

	PIPE_MSG locMsg;
	locMsg.befNum = 0;
	locMsg.lastNum = 1;
	CHECK( write(fileDesc, &locMsg, sizeof(locMsg)) != sizeof(locMsg) );
	printf("2-3\n");
	
	CHECK( close(fileDesc) != -1 );
	printf("2-4\n");

	enableSem( 2 );	// povoli prvy semafor
}

// pocka na poslednu spravu od childa
void waitForLastChildMsg(int countNum) {
	waitSem(0);	// rodic caka na semafor 0 napevno

	// vytvor fifo
	umask(0);
	CHECK( (mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR) != -1) || (errno == EEXIST) );
	
	// otvor fifo
	int fileDesc;
	CHECK( (fileDesc = open(FIFO_NAME, O_CREAT | O_RDONLY)) != -1 );

	// zmaz fifo z pamate
	unlink(FIFO_NAME);

	// readFifo
	PIPE_MSG locMsg;
	CHECK( read(fileDesc, &locMsg, sizeof(locMsg)) );

	// pritnf result
	printf("parent: posledny(%d) clen Fibonacciho postupnosti: %d\n", countNum, locMsg.lastNum);

	// closeFifo
	CHECK( close(fileDesc) != -1 );
}

// prisiel signal na ukoncenie, preposli tento signal vsetkym child procesom
void sendTermSignalToChildren(pid_t * childrenPids, int childrenNum) {
	int childId;
	for(childId = 0; childId < childrenNum; childId++) {
		kill(childrenPids[childId], SIGUSR1);
	}
}

// pocka na ukoncenie vsetkych child procesov
void waitForChildrenTermination(int childrenNum) {
	int childId;
	for(childId = 0; childId < childrenNum; childId++) {
		wait(NULL);
	}
}

// vypise nacitane argumenty
void printArgs(ARGS * args) {
	printf("Ncitane parametre: \n");
	printf("\thelp: %d\n", args->h);
	printf("\tcountArgStr: %s\n", args->countArgStr);
	printf("\tcountArg: %d\n", args->countArg);
}

// vypise help
void printHelp(FILE* stream) {
	fprintf(stream, "Usage: parametre [-h | --help] -c|--count <countArg>\n");
    fprintf(stream, "Prepinace:\n");
    fprintf(stream, " -h, --help  vypise help\n");
    fprintf(stream, " -c, --count <countArg> vytvori countArg pocet semaforov\n");
}

// vypis ehelp a ukonci program
void printHelpAndExit(FILE* stream, int exitCode) {
	 printHelp(stream);
    exit(exitCode);
}

// nacita vstupne prepinace/argumenty
void parseArguments(int argc, char * argv[], ARGS * args) {
	int opt;
    struct option longOptions[] = {
        {"help"	, no_argument, NULL, 'h'},
        {"count", no_argument, NULL, 'c'},
        {NULL	, 0			 , NULL, 0  }
    };
    
    // nacitaj parametre s prepinacom
    do {
        opt = getopt_long(argc, argv, ":hc:", longOptions, NULL);
        switch(opt) {
            case 'h':
				args->h = SET;
				printHelp(stdout);
                break;
            case 'c':
				args->c = SET;
				args->countArgStr = optarg;
                break;
            case ':':
                fprintf(stderr, "Chyba povinny argument prepinaca: %c\n", optopt);
                printHelpAndExit(stderr, EXIT_FAILURE);
            case '?':
                fprintf(stderr, "Neznamy prepinac: %c\n", optopt);
                printHelpAndExit(stderr, EXIT_FAILURE);
                break;
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
	if (args->c == UNSET) {
		printf("CHYBA: nebol zadany prepinac -c\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}

	if (sscanf(args->countArgStr, "%d", &args->countArg) != 1) {
		printf("CHYBA: argument prepinaca -c musi byt cislo\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
}

// zresetuje hodnoty argumentov
void resetArguments(ARGS * args) {
	args->h = UNSET;
	args->c = UNSET;
	args->countArgStr = "default";
	args->countArg = 0;
}
