// PARSOVANIE / VALIDACIA ARGUMENTOV

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

//typ reprezentujuci hodnotu prepinaca (zadany, nezadany)
typedef enum {
	UNSET = 0,
	SET = 1
} OPTION;

//struktura reprezentujuca vstupne argumenty
typedef struct {
	OPTION h; //volba h (help) bez argumentu
	OPTION a; //volba a bez argumentu
	OPTION b; //volba b s povinnym argumentom
	char*  bArg; //argument volby b
	OPTION c; //volba c s povinnym argumentom
	char*  cArg;    //argument volby c ako retazec (nastavi sa pri parsovani)
	int    cIntArg; //argument volby c ako cislo   (nastavi sa pri validovani)
	OPTION d; //volba d s volitelnym argumentom
	char*  dArg; //argument volby d
} ARGS;

//parsovanie argumentov
void parseArgs(int argc, char * argv[], ARGS * args) {
	int opt;

	//inicializacia parametrov 
	args->h = UNSET;
	args->a = UNSET;
	args->b = UNSET;
	args->bArg = NULL;
	args->c = UNSET;
	args->cArg = NULL;
	args->d = UNSET;
	args->dArg = NULL;

	struct option long_options[] = {
		{"help",0,NULL,'h'},
		{"alpha",0,NULL,'a'},
		{"beta",1,NULL,'b'},
		{"gamma",1,NULL,'c'},
		{"delta",2,NULL,'d'},
		{0,0,0,0}
	};
	int option_index = 0;
	do {
		opt = getopt_long(argc, argv, ":hab:c:d::",long_options,&option_index);
		printf("Debug:    opt = '%c'(%3d), optopt='%c', optarg=%s, optind=%d, opterr=%d\n",
		                         opt, opt, optopt, optarg, optind, opterr);
		//switch DOPLNTE
		
		switch(opt) {
			case 'h':
				args->h = SET;
				break;
			case 'a':
				args->a = SET;
				break;
			case 'b':
				args->b = SET;
				args->bArg = optarg;
				break;
			case 'c':
				args->c = SET;
				args->cArg = optarg;
				break;
			case 'd':
				args->d = SET;
				args->dArg = optarg;
				break;
			case ':':
				fprintf(stderr, "Nebol zadany argument prepinaca -%c\n",optopt);
				printHelpAndExit(stderr,EXIT_FAILURE);
			case '?':
				fprintf(stderr,"Neznama volba -%c\n",optopt);
				printHelpAndExit(stderr, EXIT_FAILURE);
			default:
				break;
		}
		
		
	}while(opt != -1);

	printf("Debug: zvysne argumenty:\n");
	while(optind < argc ) {
		printf("Debug:    non-option ARGV-element: %s\n", argv[optind++]);
	}
}

//validacia argumentov
void validateArgs(ARGS * args) {

	//ak nebol zadany argument prepinaca d, tak nastav jeho hodnotu na prazdny retazec
	//DOPLNTE
	if(args->d == SET && args->dArg == NULL) args->dArg = (char*) "nenastaveny";
	
	//over ci bol zadany povinny prepinac c
	//DOPLNTE
	if(args->c != SET) {
		fprintf(stderr, "Nebol zadany prepinac c\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}	
	//skonvertuj argument prepinaca c na cislo
	//DOPLNTE
	if(sscanf(args->cArg,"%d",&args->cIntArg) <= 0) {
		printErr("Argument prepinaca nie je cislo\n");
		printHelpAndExit(stderr, EXIT_FAILURE);	
	}

}

