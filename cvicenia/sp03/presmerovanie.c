#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

typedef enum {
	UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
	OPTION help;
	char * outputFile;
} SETTINGS;

void printHelpAndExit(FILE * stream, int exitCode) {
	fprintf(stream, "Usage: presmerovanie (-h | vystupny_subor)\n");
	fprintf(stream, "Program vypise do suboru vystupny_subor procesy pouzivatela\n");
	fprintf(stream, "Prepinac -h vypise help\n");
	exit(exitCode);
}

void parseArguments(int argc, char*argv[], SETTINGS * settings) {
	int opt;
	
	settings->help = UNSET;
	settings->outputFile = NULL;
		
	do {
		opt = getopt(argc, argv, ":h");		
		
		switch(opt) {
        case 'h': 
			settings->help = SET;
			break;
		case ':':
			fprintf(stderr, "CHYBA: nezadany argument volby '%c'\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
			break;
		case '?':
			fprintf(stderr, "CHYBA: neznama volba '%c'\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
			break;
		}
	}while(opt != -1);

	if( settings->help == UNSET ) {		
		if( argv[optind] == NULL ) {
			fprintf(stderr, "CHYBA: nezadany vystupny subor\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
		}
		
		if( argv[optind+1] != NULL ) {
			fprintf(stderr, "CHYBA: prilis vela parametrov\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
		}
			
		settings->outputFile = argv[optind];
	}
}

int main(int argc, char * argv[]) {
	SETTINGS settings;
	int file, sout; 
	char *args[] = {
		"ps",
		"u",
		NULL,
	};
	
	parseArguments(argc, argv, &settings);
	         
	if( settings.help == SET ) {
		printHelpAndExit(stdout, EXIT_SUCCESS);
	}

	CHECK(file = open(settings.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0600));

	CHECK(close(STDOUT_FILENO) + 1);

	CHECK(sout = dup(file));
		
	CHECK(close(file) + 1);

	printf("Zoznam aktualnych procesov:\n");

	fflush(stdout);
		
	execvp(args[0], args);
	perror("exec");	
	return EXIT_FAILURE;
}
	
