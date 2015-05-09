#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>

typedef enum{
    UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
    char *startDir;
    int e;
    int n;
} ARGS;

void initArguments(ARGS *args) {
    args->e = UNSET;
    args->n = UNSET;
    args->startDir = NULL;
}
void parseArguments(int argc,char *argv[], ARGS *args) {
    initArguments(args);

    struct option long_opts[] = {
        {
            "exist",0,NULL,'e'
        },
        {
            "not_exist",0,NULL,'n'
        }
    };
    int opt;
    int opt_index = 1;
    do {
        opt = getopt_long(argc,argv,"en",long_opts, &opt_index);

        switch(opt) {
            case 'e':
                args->e = SET;
                break;
            case 'n':
                args->n = SET;
                break;
            case '?':
                fprintf(stderr,"Neznama volba -%c\n",optopt);
            default:
                break;
        }
    } while(opt != -1);

    while(optind < argc) {
        args->startDir = argv[optind];
        break;
    }
        
}

void printOptions(ARGS * args) {
	printf("vypis parametrov programu:\n");
	printf("    e: %s\n", args->e==SET?"SET":"UNSET");
	printf("    n: %s\n", args->n==SET?"SET":"UNSET");
	printf("dir  : %s\n", args->startDir != NULL ? args->startDir : "NULL");
}


void validateArgs(ARGS *args){
    if(args->startDir == NULL) {
        fprintf(stderr,"Zadajte pociatocny priecinok\n");
        exit(EXIT_FAILURE);
    }
    if(args->e == SET && args->n == SET) {
        fprintf(stderr,"Vyberte si iba jednu volbu z -e alebo -n\n");
        exit(EXIT_FAILURE);
    }
}
int main(int argc, char *argv[]) {
    ARGS args;
    parseArguments(argc,argv,&args);
    validateArgs(&args);
    //printOptions(&args);

    return 0;
}
