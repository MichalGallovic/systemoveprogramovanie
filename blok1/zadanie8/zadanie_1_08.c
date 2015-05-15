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
#include <sys/sendfile.h>

//makro pre jednoduchsiu kontrolu navratovej hodnoty volania funkcie a vypis popisu chyby
#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }


// makra, struktury, konstanty, ...
typedef enum {
    UNSET=0,
    SET=1
} OPTION;

typedef struct {
    OPTION h;
    OPTION c;
    char * cNumArg;
    int cNum;
} ARGS;

typedef enum {
	MAKE_DIR = 0,
	COPY_TO_DIR,
	BACKUP_ACT_DIR,
	DISPLAY_DIR_CONTENT,
	TERMINATION
} MENU_CHOICE;

#define DEFAULT_DIR_NAME "backup_directory"	// nazov adresara, ktory ma byt default vytvoreny pre potreby tohto zadania
#define MAX_FILES_NUM 10	// maximalny pocet nacitanych suborov pre kopirovanie do adresara

typedef struct {
	MENU_CHOICE choice;								// volba z menu
	char makeDirName[PATH_MAX];						// nazov adresara, ktory sa ma vytvorit
	char copyFilesNames[MAX_FILES_NUM][PATH_MAX];	// nazvy suborov, ktore sa maju skopirovat
	char displayContentDirName[PATH_MAX];			// nazov adresara, ktoreho obsah sa ma rekurzivne zobrazit
} USER_INPUT;

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
// zresetuje hodnoty argumentov
void resetArguments(ARGS * args);
// rekurzivne do hlbky vypise vsetky regularne vyrazy v danom priecinku
void displayDirContentRec(char * directory);
// vytvori adresar
void makeDir(char * dirName);
// pozrie ci existuje defaultny adresar, ak nie ho vytvori
void createDefaultDirectory();
// zalohuje data z adresara 
void backupActDir();

// vypise menu
void printMenu();
// nacita vyber + suvisiace vstupy od uzivatela
void getUserInput(USER_INPUT * userInput);
// vykona ulohu podla vyberu z menu
void processChosenAction(USER_INPUT * userInput);
// nacita vstup od usera
void getInput(char * input, size_t size);
// vyprazdni stdin
void resetStdin();

int main(int argc, char * argv[]) {
	ARGS args;
	USER_INPUT userInput;	// obsahuje nacitane vstupy od uzivatela

	resetArguments(&args);
	parseArguments(argc, argv, &args);
	validateArguments(&args);

	int counter;
	for(counter = 0; ; counter++) {
		if (args.c == SET && counter < args.cNum) break;
		printMenu();						// vypise menu
		getUserInput(&userInput);			// nacita vyber + suvisiace vstupy od uzivatela
		processChosenAction(&userInput);	// vykona ulohu podla vyberu z menu
	}
	return EXIT_SUCCESS;
}


/************************ definicie fcii *****************************/

// vypise menu
void printMenu() {
	printf("\t\t*** Menu ***\n");
	printf("\t 1 - Vytvorit adresar\n");
	printf("\t 2 - Kopirovat subory do adresara\n");
	printf("\t 3 - Zalohovat subory z aktualneho adresara\n");
	printf("\t 4 - Zobrazit obsah adresara\n");
	printf("\t 5 - Koniec\n\n");
}

// vyprazdni stdin
void resetStdin() {
	int c;
	while( (c = getchar()) != EOF && c != '\n') continue;
}


// nacita vyber + suvisiace vstupy od uzivatela
void getUserInput(USER_INPUT * userInput) {
	printf("Zadajte 1-5 pre volbu polozky z menu\n");

	int c;
	while( scanf("%d", &c) != 1) {
		resetStdin();	// resetuje stdin
		printf("Zadajte cislo od 1 do 5\n");	
	}
	
	resetStdin();

	switch(c) {
		case 1:		// vytvorit adresar
			{
			char path[PATH_MAX];
			printf("Zadajte nazov adresaha, ktory chete vytvorit: \n");
			fgets(path, sizeof(path), stdin);
			
			userInput->choice = MAKE_DIR;
			strncpy(userInput->makeDirName, path, sizeof(path));
			userInput->makeDirName[strlen(path)-1] = '\0';		// -1 kvoli tomu, ze fgets nacita aj newline
			}
			break;
		case 2:		// kopirovat subory do adresara
			userInput->choice = COPY_TO_DIR;
			break;
		case 3:		// zalohovat subory z akt priecinka
			userInput->choice = BACKUP_ACT_DIR;
			break;
		case 4:		// zobrazit obsah adresara
			{
			char path[PATH_MAX];
			printf("Zadajte nazov adresaha, ktoreho obsah chcete zobrazit: \n");
			fgets(path, sizeof(path), stdin);
		
			userInput->choice = DISPLAY_DIR_CONTENT;
			strncpy(userInput->displayContentDirName, path, sizeof(path));
			userInput->displayContentDirName[strlen(path)-1] = '\0';		// -1 kvoli tou ze fgets nacita aj newline
			}
			break;
		case 5:		// koniec
			userInput->choice = TERMINATION;
			break;
	}
}

// vykona ulohu podla vyberu z menu
void processChosenAction(USER_INPUT * userInput) {
	if (userInput->choice == TERMINATION) exit(EXIT_SUCCESS);	

	pid_t pid;
	CHECK( (pid = fork()) != -1 ); // vytvor detsky proces
	switch(pid) {
		case 0:	// child
			switch(userInput->choice) {	// vykonaj prislusnu ulohu
				case MAKE_DIR:
					makeDir(userInput->makeDirName);
					printf("Adresar vytvoreny.\n");
				case COPY_TO_DIR:
					createDefaultDirectory();
					//copyFilesToDirectory(userInput->copyFilesNames);
					printf("Toto je obdobne ako backup adresara, iba sa dorobi nacitanie suborov - som lazy\n");
					break;
				case BACKUP_ACT_DIR:
					createDefaultDirectory();
					backupActDir();
					printf("backup hotovy.\n");
					break;
				case DISPLAY_DIR_CONTENT:
					displayDirContentRec(userInput->displayContentDirName);
					break;
				case TERMINATION:
					break;
			}	
			exit(EXIT_SUCCESS);
		default:	// parent
			wait(NULL);	// pockaj kym skonci detsky proces
			return;
	}
}

// zalohuje aktualny adresar
void backupActDir() {
	char path[PATH_MAX];
	CHECK( getcwd(path, sizeof(path)) != NULL );

	DIR * dirStream;
	CHECK( (dirStream = opendir(path)) != NULL );
	
	memset(path, 0, sizeof(path));
	struct dirent * dirEntry;
	struct stat info;
	char numStr[5];
	int backupFileDesc;
	int origFileDesc;

	int verNum;
	const int MAX_VER_NUM = 5;
	while( (dirEntry = readdir(dirStream)) != NULL) {
		CHECK( lstat(dirEntry->d_name, &info) != -1 );
		if (S_ISREG(info.st_mode) != 1) continue;

		for(verNum = 0; verNum < MAX_VER_NUM; verNum++) {
			// vytovri nazov = origNazov.verzia
			sprintf(numStr, "%d", verNum);

			memset(path, 0, sizeof(path));
			strcat(path, DEFAULT_DIR_NAME);
			strcat(path, "/");
			strcat(path, dirEntry->d_name);
			strcat(path, ".");
			strcat(path, numStr);

			//printf("path: %s\n", path);
			// zisti ci taky subor uz existuje v default directory
			if (access(path, F_OK) == 0) continue;		// ak k takemu suboru vie pristup - existuje

			CHECK( (origFileDesc = open(dirEntry->d_name, O_RDONLY, 0660)) != -1 );		// otvor file na citanie
			CHECK( (backupFileDesc = open(path, O_CREAT | O_WRONLY, 0660)) != -1 );		// otvro file na zapis
			CHECK( sendfile(backupFileDesc, origFileDesc, NULL, info.st_size) != -1);	// posli obsah

			// pozatvaraj deskritpry
			CHECK( close(origFileDesc) != -1 );
			CHECK( close(backupFileDesc) != -1 );
			break;
		}
	}

}

// vytvori adresar
void makeDir(char * dirName) {
	CHECK( mkdir(dirName, 0777) != -1);
}

// pozrie ci existuje defaultny adresar, ak nie ho vytvori
void createDefaultDirectory() {
	char path[PATH_MAX];
	CHECK( getcwd(path, sizeof(path)) != NULL );
	strcat(path, "/");
	strcat(path, DEFAULT_DIR_NAME);

	struct stat info;
	CHECK( (lstat(path, &info) != -1) || (errno == ENOENT) );
	if (errno == ENOENT) makeDir(path);
}

// rekurzivne do hlbky vypise vsetky regularne vyrazy v danom priecinku
void displayDirContentRec(char * directory) {
	char path[PATH_MAX];	// pri rekurziach pracuj vzdy s lokalnou kopiou
	DIR * dirStream;
	printf("directory: %s\n", directory);
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

// vypise nacitane argumenty
void printArgs(ARGS * args) {
	printf("Ncitane parametre: \n");
	printf("\thelp: %d\n", args->h);
	printf("\tcount: %d\n", args->c);
	printf("\tcountNum: %d\n", args->cNum);
}

// vypise help
void printHelp(FILE* stream) {
	fprintf(stream, "Usage: parametre [-h | --help][-c|--count][<count>]\n");
    fprintf(stream, "Prepinace:\n");
    fprintf(stream, " -h, --help  vypise help\n");
    fprintf(stream, " -c, --count  umozni vykonat fcie obmedzeny pocet krat\n");
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
                args->cNumArg = optarg;
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
	if (args->c == UNSET) return;

	if (scanf(args->cNumArg, "%d", args->cNum) != 1) {
		printf("CHYBA: argument prepinaca -c musi byt cislo\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
}

// zresetuje hodnoty argumentov
void resetArguments(ARGS * args) {
	args->h = UNSET;
	args->c = UNSET;
	args->cNumArg = "default";
	args->cNum = 0;
}

