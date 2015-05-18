#include "barier_shm_sem.h"

typedef enum {
    UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
    OPTION h;
    OPTION n;
    OPTION t;
    char * nproc;
    int n_proc;
    char *text;
} ARGS;

int quit = 1;

void printHelpAndExit(FILE* stream, int exitCode){
    fprintf(stream, "Usage: parametre [-h] [-n <N>] [-t <text>]\n");
    fprintf(stream, "Zadanie 2-03\n");
    fprintf(stream, "Prepinace:\n");
    fprintf(stream, " -h, --help   vypise help\n");
    fprintf(stream, " -n, pocet procesov\n");
    fprintf(stream, " -t, text, ktory maju procesy zapisovat\n");
    exit(exitCode);
}

void parseArgs(int argc, char * argv[], ARGS * args) {
    int opt;

    args->h = UNSET;
    args->n = UNSET;
    args->t = UNSET;
    args->text = NULL;
    args->nproc = NULL;
    args->n_proc = 4;

    static struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {0, 0, 0, 0}
    };
    int option_index = 0;

    do {
        opt = getopt_long(argc, argv, ":hn:t:", long_options, &option_index);
        switch (opt) {
            case 'h':
                args->h = SET;
                printHelpAndExit(stderr, EXIT_SUCCESS);
                break;
            case 'n':
                args->n = SET;
                args->nproc = optarg;
                break;
            case 't':
                args->t = SET;
                args->text = optarg;
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
    if(args->n == SET){
        if(sscanf(args->nproc, "%d", &args->n_proc) <= 0 ) {
            fprintf(stderr, "Argument prepinaca -n nie je cislo!\n");
            printHelpAndExit(stderr, EXIT_FAILURE);
        }
        if(args->n_proc < 4){
            fprintf(stderr, "Argument prepinaca -n musi byt vacsi ako 4\n");
            printHelpAndExit(stderr, EXIT_FAILURE);
        }   
    }

    if(args->t == SET) {
        if(strlen(args->text) == 0) {
            fprintf(stderr, "Agument prepinaca text nemoze byt prazdy string");
            printHelpAndExit(stderr, EXIT_FAILURE);
        }
    }
}


void ChildSignalHandler(int signal_num){
    quit = 0;
}

void doWork(char *text){
    char buf[4096];
    char buf1[4096];
    char temp[20];
    struct sigaction sa;
    buf[0] = '\0';

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &ChildSignalHandler;
    CHECK(sigaction(SIGUSR1, &sa, NULL) ==  0);

    lockMutex();
    read_((char*)&buf1); //precita obsah pamate
    sprintf((char*)&temp, "PID: %u %s\n", getpid(), text); ///vytvori string z PID
    fprintf(stderr, "%s", temp);
    strcat((char*)&buf, (char*)&buf1); //spoji dokopy
    // fprintf(stderr, "@SHARED:%s", buf);
    strcat((char*)&buf, (char*)&temp);
    fprintf(stderr, "2SHARED:%s", buf);
    write_((char*)&buf, strlen(buf));
    //sleep(1);
    unlockMutex();
}
////vracia poziciu nasledujuceho v poli, ak je posledny tak prveho
//int getNext(int* array, int position, int max){
//    if(position == max)
//        return array[0];
//    else 
//        return array[position];
//}
void createProc(int number, pid_t* pid_array, char *text){
   int i = 0;
   pid_t pid;
   unlockMutex();

   for(i = 0; i < number; ++i){
       switch(pid = fork()){
           case 0:
               doWork(text);
               exit(EXIT_SUCCESS);
           case -1:
               perror("fork");
               exit(EXIT_FAILURE);
           default:
               pid_array[i] = pid;
               break;
       }
   }

}

int main(int argc, char * argv[]){
    ARGS args;
    pid_t *pid;
    int i = 0;
    char buf[4096];

    parseArgs(argc, argv, &args);
    validateArgs(&args);

    init(args.n_proc);

    pid = (pid_t*)malloc(sizeof(pid_t) * args.n_proc);  

    createProc(args.n_proc, pid, args.text);

    ///KILLOVANIE deti
    sigset_t set;
    int sig_num;
//
//    sigemptyset(&set);
//    sigaddset(&set, SIGINT);
//    CHECK(sigprocmask(/*SIG_BLOCK*/ SIG_SETMASK, &set, NULL) == 0);
//    CHECK(sigwait(&set, &sig_num) == 0);
//
//    for(i = 0; i < args.n_proc; i++){
//        CHECK(kill(pid[i], SIGUSR1) == 0 );
//    }

    for(i = 0; i < args.n_proc; i++){
        CHECK(wait(NULL) != -1);    
    }

    fprintf(stderr,"\nProcesy zapisali do SHM:\n%s",buf);
    teardown();
    //free(pid);
    return EXIT_SUCCESS;
}
