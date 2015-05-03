#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

typedef enum {
	UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
	OPTION help;
	char * symbolicLink; 
} SETTINGS;

void printHelpAndExit(FILE * stream, int exitCode) {
	fprintf(stream, "Usage: linkInfo (-h | linka)\n");
	fprintf(stream, "Program vypise informacie o linke a jej ciely\n");
	fprintf(stream, "Prepinac -h vypise help\n");
	exit(exitCode);
}

void parseArguments(int argc, char*argv[], SETTINGS * settings) {
	int opt;
	
	settings->help = UNSET;
	settings->symbolicLink = NULL;
		
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

void validateArguments(SETTINGS * settings) {
    struct stat info;

	CHECK(lstat(settings->symbolicLink, &info) + 1);
	
	if(!S_ISLNK(info.st_mode)){
		fprintf(stderr, "Zadany argument nie je symlink\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
}

void printSymbolicLinkInfo(SETTINGS * settings) {
    struct stat info;

	CHECK(lstat(settings->symbolicLink, &info) + 1);
	printf("Name: %s\n", settings->symbolicLink);
	printf("Size: %d bytes\n", (int)info.st_size);
	
	char * link_target = (char *)malloc((info.st_size + 1) * sizeof(char));
	CHECK(readlink(settings->symbolicLink, link_target, info.st_size));
	link_target[info.st_size] = '\0';

	CHECK(stat(settings->symbolicLink, &info) + 1);
	printf("Name of link target: %s\n", link_target);
	printf("Size of link target: %d bytes\n", (int)info.st_size);
	free(link_target);
}

int main(int argc, char * argv[]) {
	SETTINGS settings;
	
	parseArguments(argc, argv, &settings);
	         
	if( settings.help == SET ) {
		printHelpAndExit(stdout, EXIT_SUCCESS);
	}
	
	validateArguments(&settings);
	
	printSymbolicLinkInfo(&settings);

	return EXIT_SUCCESS;
}

