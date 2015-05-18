#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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

//vypis help-u a ukoncenie programu
void prinHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage: parametre [-h] [-a | alpha] [(-b | --beta) argB] (-c | --gamma) argC [(-d | delta) [argD]]\n");
	fprintf(stream, "Program vypise nastavenie svojich vstupnych argumentov\n");
	fprintf(stream, "Prepinace:\n");
	fprintf(stream, " -h, --help   vypise help\n");
	fprintf(stream, " -a, --alpha  popis vyznamu parametra .....\n");
	fprintf(stream, " -b, --beta   popis vyznamu parametra .....\n");
	fprintf(stream, " -c, --gamma  popis vyznamu parametra .....\n");
	fprintf(stream, " -d, --delta  popis vyznamu parametra .....\n");
	exit(exitCode);
}

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

	//parsovanie argumentov
	printf("Debug: parsovanie argumentov:\n");
	do {
		opt = getopt(argc, argv, "NAHRADTE SPRAVNYM RETAZCOM");
		printf("Debug:    opt = '%c'(%3d), optopt='%c', optarg=%s, optind=%d, opterr=%d\n",
		                         opt, opt, optopt, optarg, optind, opterr);
		//switch DOPLNTE
		
		
		
		
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
	
	//over ci bol zadany povinny prepinac c
	//DOPLNTE
	
	//skonvertuj argument prepinaca c na cislo
	//DOPLNTE
}

//debug vypis zadanych argumentov
int printOptions(ARGS * args) {
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

	if( args.h == SET ) {
		prinHelpAndExit(stdout, EXIT_SUCCESS);
	}

	validateArgs(&args);

	printOptions(&args); //tu by bola funkcionalita programu

	return EXIT_SUCCESS;
}
