/***************************************
==>zadanie_1_20.c <==
Napiste program, ktory spusti programy <prog1> a <prog2> zadane parametrami "--program1" a "--program2" (bez argumentov).
Programy <prog1> aj <prog2> nech vypisuju obsah suboru <file> (zadaneho volitelnym parametrom programu "-f" alebo "--file")
na standardny vystup. Pritom oba vypisy musia byt koordinovane signalmi, aby sa vzajomne nemiesali: riadky vypisu sa budu
pravidelne striedat podla uvedeneho vzoru vystupu. Ak nie je zadany argument <file>, vytvorte detsky proces, ktory na standardny
vystup vypise zoznam vsetkych regularnych suborov v aktualnom priecinku (rekurzivne do hlbky); a tento proces zabezpeci
nacitanie volby pouzivatela vzhladom na parameter "-f". Rodicovsky proces nech zatial caka na spracovanie vstupu od pouzivatela
detskym procesom. Identifikatorom procesov na vystupe nech je ich PID. Program musi prehladavat aktualny adresar rekurzivne
pomocou API funkcii jadra systemu.
Volanie programu: zadanie.elf --program1 <prog1> --program2 <prog2> [-f <file>]
Priklad vystupu: <proces1>: <1. riadok suboru <file>>
                           <proces2>: <1. riadok suboru <file>>
                           <proces1>: <2. riadok suboru <file>>
                           <proces2>: <2. riadok suboru <file>>
                           ...
********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/wait.h>


typedef enum{
    UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
        OPTION p1;
        OPTION p2;
        OPTION f;
	const char* pr1;

	const char* pr2;

	const char* nf;


} ARGS;

void initArguments(ARGS *args) {
    args->p1 = UNSET;
    args->p2 = UNSET;
    args->f = UNSET;

    args->pr1 = NULL;
    args->pr2 = NULL;
    args->nf = NULL;
}

/* nazov prveho a druheho programu */
void child(int childId) {
	sleep(1);
	printf("child %d\n", childId);
}



void parseArguments(int argc,char *argv[], ARGS *args) {
    initArguments(args);


    /* Retezec obsahujici kratke volby */
  const char *short_options = "p:r:f:";

  /* Pole struktur s dlouhymi volbami */
  const struct option long_options[] = {
    { "program1",    1, NULL, 'p' },
    { "program2",  1, NULL, 'r' },
    { "file",   1, NULL, 'f' },
    { NULL,      0, NULL,  0  }
  };

    int opt;
    int opt_index = 1;
    do {
        opt = getopt_long(argc,argv,short_options,long_options, &opt_index);

        switch(opt) {
            case 'p':
		args->p1 = SET;
              	args->pr1 = optarg;  
                break;
            case 'r':
		args->p2 = SET;
                args->pr2=optarg;
                break;
	    case 'f':
		args->f = SET;
		args->nf=optarg;
		break;
            case '?':
                fprintf(stderr,"Neznama volba -%c\n",optopt);
            default:
                break;
        }
    } while(opt != -1);

    /*while(optind < argc) {
        args->startDir = argv[optind];
        break;
    }*/
        
}

void validateArgs(ARGS *args){
    if(args->p1 == UNSET || args->p2 == UNSET || args->f == UNSET) {
        fprintf(stderr,"Niektory z povinnych argumentov nebol zadany\n");
        exit(EXIT_FAILURE);
    }
}

void printOptions(ARGS * args) {
	printf("vypis parametrov programu:\n");
	printf("%s -  %s -  %s\n",args->pr1,args->pr2,args->nf);
	/*printf("dir  : %s\n", args->p1 != NULL ? args->p1 : "NULL");
	printf("dir  : %s\n", args->p2  != NULL ? args->p2 : "NULL");
	printf("dir  : %s\n", args->f != NULL ? args->f : "NULL");*/
}


int main(int argc, char *argv[])
{

	ARGS args;
	parseArguments(argc,argv,&args);
   	validateArgs(&args);
	printOptions(&args);

	pid_t pid = fork();

	if(pid == 0){
		char s[30];
		sprintf(s, "%d", getppid());
		char *argv1[] = {"test", args.nf,  s, NULL};
		//printf("dieta\n");
		execv(args.pr1,argv1);
	}
	
	else{

		char s[30];
		sprintf(s, "%d", pid);
		char *argv2[] = {"test", args.nf, s, NULL};
		//printf("rodic\n");
		execv(args.pr2,argv2);
		waitpid(pid,0,0);
	}
	
	
	
	return EXIT_SUCCESS;
}
