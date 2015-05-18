#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

//makro pre jednoduchsiu kontrolu navratovej hodnoty volania funkcie a vypis popisu chyby
#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

//typ reprezentujuci hodnotu prepinaca (zadany, nezadany)
typedef enum {
	UNSET = 0,
    SET = 1
} OPTION;

//nastavenia programu (podla vstupnych parametrov)
typedef struct {
	OPTION help;
	char * symbolicLink; //cesta ku zadanej symbolickej linke
} SETTINGS;

//vypis help-u a ukoncenie programu
void printHelpAndExit(FILE * stream, int exitCode) {
	fprintf(stream, "Usage: linkInfo (-h | linka)\n");
	fprintf(stream, "Program vypise informacie o linke a jej ciely\n");
	fprintf(stream, "Prepinac -h vypise help\n");
	exit(exitCode);
}

//parsovanie vstupnych argumentov programu
void parseArguments(int argc, char*argv[], SETTINGS * settings) {
	int opt;
	
	//inicializacia nastaveni na defaultne hodnoty
	settings->help = UNSET;
	settings->symbolicLink = NULL;
		
	//parsovanie argumentov zacinajucich pomlckov		
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

    //parsovanie argumentov bez pomlcky
	if( settings->help == UNSET ) {		
		if( argv[optind] == NULL ) {
			fprintf(stderr, "CHYBA: nezadana vstupna linka\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
		}
		
		if( argv[optind+1] != NULL ) {
			fprintf(stderr, "CHYBA: prilis vela parametrov\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
		}
			
		settings->symbolicLink = argv[optind];
	}
}

//validacia vstupnych argumentov programu
void validateArguments(SETTINGS * settings) {
	//doplnte valiciu, ktora overi, ci je vstupny argument symbolicka linka
}

//Vypisanie informacii o symbolickej linke a jej cieli (hlavna funkcionalita programu)
void printSymbolicLinkInfo(SETTINGS * settings) {

	//doplnte vypisanie informacie o linke

	//doplnte vypisanie informacii o ciele linky

}

int main(int argc, char * argv[]) {
	SETTINGS settings;
	
	parseArguments(argc, argv, &settings);
	         
	if( settings.help == SET ) {
		printHelpAndExit(stdout, EXIT_SUCCESS);
	}
	
	validateArguments(&settings);
	
	printSymbolicLinkInfo(&settings); //hlavna funkcionalita programu

	return EXIT_SUCCESS;
}

