#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

typedef enum {
	UNSET = 0,
	SET = 1
} OPTION;

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

void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage: parametre [-h] [-a | --alpha] [(-b | --beta) argB] (-c | --gamma) argC [(-d | --delta) [argD]]\n");
	fprintf(stream, "Program vypise nastavenie svojich vstupnych argumentov\n");
	fprintf(stream, "Prepinace:\n");
	fprintf(stream, " -h, --help   vypise help\n");
	fprintf(stream, " -a, --alpha  popis vyznamu parametra .....\n");
	fprintf(stream, " -b, --beta   popis vyznamu parametra .....\n");
	fprintf(stream, " -c, --gamma  popis vyznamu parametra .....\n");
	fprintf(stream, " -d, --delta  popis vyznamu parametra .....\n");
	exit(exitCode);
}

void parseArgs(int argc, char * argv[], ARGS * args) {
	int opt;

	args->h = UNSET;
	args->a = UNSET;
	args->b = UNSET;
	args->bArg = NULL;
	args->c = UNSET;
	args->cArg = NULL;
	args->d = UNSET;
	args->dArg = NULL;
	
	static struct option long_options[] = {
                   						  {"help", 0, NULL, 'h'},
                   						  {"alpha", 0, NULL, 'a'},
                   						  {"beta", 1, NULL, 'b'},
                   						  {"gamma", 1, NULL, 'c'},
                   						  {"delta", 2, NULL, 'd'},
                   						  {0, 0, 0, 0}
               							  };
	int option_index = 0;

	do {
		opt = getopt_long(argc, argv, ":hab:c:d::", long_options, &option_index);

		//opt = getopt(argc, argv, ":hab:c:d::");
	//	printf("Debug:    opt = '%c'(%3d), optopt='%c', optarg=%s, optind=%d, opterr=%d\n",
	//	                        opt, opt, optopt, optarg, optind, opterr);
		switch (opt) {
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
		case '?': 	
			fprintf(stderr,"Neznama volba -%c\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		case ':': 	
			fprintf(stderr, "Nebol zadany argument prepinaca '-%c'\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		default:
			break;
		}
		
	} while(opt != -1);

	while(optind < argc ) {
		printf("Debug:    non-option ARGV-element: %s\n", argv[optind++]);
	}
}

void validateArgs(ARGS * args) {

	if(args->d == SET  && args->dArg == NULL )  args->dArg = (char*) '\0';
	if(args->c != SET) {
		fprintf(stderr, "Nebol zadany povinny prepinac c\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
	if(sscanf(args->cArg, "%d", &args->cIntArg) <= 0 ) {
		fprintf(stderr, "Argument prepinaca -c nie je cislo!\n");
		printHelpAndExit(stderr, EXIT_FAILURE);
	}
}

void printOptions(ARGS * args) {
	printf("vypis parametrov programu:\n");
	printf("    h: %s\n", args->h==SET?"SET":"UNSET");
	printf("    a: %s\n", args->a==SET?"SET":"UNSET");
	printf("    b: %s arg: %s\n", args->b==SET?"SET":"UNSET", args->bArg);
	printf("    c: %s arg: %s\n", args->c==SET?"SET":"UNSET", args->cArg);
	printf("    d: %s arg: %s\n", args->d==SET?"SET":"UNSET", args->dArg);
}

int main(int argc, char * argv[]) {
	ARGS args;

	parseArgs(argc, argv, &args);

	if( args.h == SET )		printHelpAndExit(stdout, EXIT_SUCCESS);

	validateArgs(&args);
	printOptions(&args); 

	return EXIT_SUCCESS;
}
