#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define FIFO_PATH "/tmp/fifo_zad3_ref"

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

typedef enum{
    UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
    char *startDir;
    int p;
    char *program;
    int f;
} ARGS;

void printHelpAndExit(FILE* stream, int exitCode){
    fprintf(stream, "\nUsage: parametre [-f | --file <file>] [-p | --program <prog>]\n");
    fprintf(stream, "Program vypise zoznam regularnych suborov priecinku zadaneho prepinacom -f rekurzivne. Prepinacom -p definujeme dalsi, program, ktory vypisuje s povodnym v tandeme. Ak nie je zadany argument -f vypisujeme aktualny priecinok.\n");
    exit(exitCode);
}

/** 
* @brief Vynuluje argumenty
* 
* @param args
*/
void initArguments(ARGS *args) {
    args->p = UNSET;
    args->f = UNSET;
    args->startDir = NULL;
    args->program = NULL;
}
/** 
* @brief Parsovanie agumentov
*   pvy argument pociatocny priecinok
*   e - vypise linky s existujucim cielovym suborom
*   n - vypise linky s neexist. ciel. suborom
* @param argc
* @param argv[]]
* @param args
*/
void parseArguments(int argc,char *argv[], ARGS *args) {
    initArguments(args);

    struct option long_opts[] = {
        {"program",1,NULL,'p'},
        {"file",2,NULL,'f'},
        {NULL,0,NULL,0}
    };
    int opt;
    int opt_index = 0;
    do {
        opt = getopt_long(argc,argv,":p:f::",long_opts, &opt_index);

        switch(opt) {
            case 'p':
                args->p = SET;
                args->program = optarg;
                break;
            case 'f':
                args->f = SET;
                args->startDir = argv[optind];
                break;
            case ':':
                fprintf(stderr, "Zadajte argument prepinaca -p");
                printHelpAndExit(stderr, EXIT_FAILURE);
            case '?':
                fprintf(stderr,"Neznama volba -%c\n",optopt);
                printHelpAndExit(stderr, EXIT_FAILURE);
            default:
                break;
        }
    } while(opt != -1);
    

}


/** 
* @brief Hlavna funkcia, prehlada zadany priecinok
* sanitizne dirName ak neopsahuje trailing slash
* a vypise linky na zaklade nastaveni
* 
* @param args
*/
void listDir(char *startDir, pid_t help_proces) {
    
    
    DIR *dir;
    struct dirent *entry;
    char *dirName;
    size_t dirLength = strlen(startDir);
    dirName = (char*) malloc(dirLength*sizeof(char));
    strcpy(dirName, startDir);

//ak neni v ceste priecinka na konci "/" pridame ho
    if(dirName[dirLength - 1] != '/') {
        dirName[dirLength] = '/';
        dirName[dirLength + 1] = '\0';
        dirLength++;
    }
    
    CHECK((dir = opendir(dirName)) != NULL);

    struct stat st;
    char file[PATH_MAX + 1];
    strcpy(file, dirName);
    pid_t master_parent_pid = getpid();
    
    sigset_t set;
    int signum;
    sigaddset(&set, SIGUSR1);
    CHECK (sigprocmask(SIG_SETMASK, &set, NULL) != -1);
    
    while( (entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strncpy(file + dirLength, entry->d_name, sizeof(file) - dirLength);
            
            CHECK(lstat(file, &st) != -1);
            
            if(S_ISDIR(st.st_mode)) {
                pid_t pid = fork();
                if(pid == 0) {
                    listDir(file, help_proces);
                    exit(EXIT_SUCCESS);
                } else {
                    wait(NULL);
                }
            }
            
            if (S_ISREG(st.st_mode)) {
                fprintf(stderr, "proc: %d %s\n",(int)master_parent_pid, file);
                CHECK(kill(help_proces, SIGUSR1) != -1);
                CHECK(sigwait(&set, &signum) == 0);
            }
            
        }
    }

    closedir(dir);
}
/* Prepends t into s. Assumes s has enough space allocated
** for the combined string.
*/
void prepend(char* s, const char* t)
{
    size_t len = strlen(t);
    size_t i;

    memmove(s + len, s, strlen(s) + 1);

    for (i = 0; i < len; ++i)
    {
        s[i] = t[i];
    }
}
void startProgram(ARGS *args) {
    char *program = args->program;
    
    if(program[0] != '.' && program[1] != '/') {
        size_t len = strlen(args->program);
        program = (char*) malloc((len+2)*sizeof(char));
        strncpy(program, args->program, len);
        prepend(program, "./");
    }
    pid_t parent_pid = getppid();
    char pid_buf[20];
    sprintf(pid_buf,"%d",parent_pid);
    char *argv[] = {"/",args->startDir,pid_buf, NULL};
    execv(args->program, argv);
    perror("execlp");
}
void executeChoice(ARGS *args) {
    sigset_t set;
    int signum;
    sigaddset(&set, SIGUSR2);
    CHECK (sigprocmask(SIG_SETMASK, &set, NULL) != -1);
    pid_t pid = fork();
    switch (pid) {
    	case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            startProgram(args); 
            break;
        default:
            CHECK( sigwait(&set, &signum) == 0);
            listDir(args->startDir, pid);        
            break;
   }

}

char * getPath() {
    char buf[PATH_MAX];
    printf("Zadaj cestu k novemu priecinku: ");
    scanf("%s",buf);
    return buf;
}

void child() {

    int fifo;
    char * path = getPath();
    umask(0);

    if(mkfifo(FIFO_PATH, 0660) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    if( (fifo = open(FIFO_PATH, O_WRONLY) ) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if(unlink(FIFO_PATH) != 0) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    write(fifo, path, PATH_MAX);
    if(close(fifo) != 0 ) {
        perror("close");
        exit(EXIT_FAILURE);
    }

}


void parent(ARGS *args) {
    char path[PATH_MAX];
    int fifo;

    umask(0);
    if(mkfifo(FIFO_PATH, 0660) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    if( (fifo = open(FIFO_PATH, O_RDONLY)) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if(read(fifo,path, PATH_MAX) != PATH_MAX) {
        fprintf(stderr," Error fifo reading");
        exit(EXIT_FAILURE);
    }
    if(close(fifo) != 0 ) {
        perror("close");
        exit(EXIT_FAILURE);
    }
    
    size_t len = strlen(path);
    // clear startDir & set to new from child
    memset(args->startDir,'\0',PATH_MAX);
    strncpy(args->startDir, path, len);
}
/** 
* @brief Ziska cestu od childa - komunikacia cez FIFO
* 
* @param args
*/
void getPathFromChild(ARGS *args) {

    printf("Chces prehladavat dalsi subor? [y/n]: ");
    char choice, temp;
    choice = temp = getchar();
    // nechceme newliny
    while(temp != '\n' && temp!= EOF)
        temp = getchar();

    if(choice == 'n') exit(EXIT_SUCCESS);
    
    pid_t pid;
    
    pid = fork();

    switch(pid) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            child();
            exit(EXIT_SUCCESS);
        default:
            break;
    }
    parent(args);
}
void validateArgs(ARGS *args){
    if(args->f == UNSET) {
        args->startDir = "./";
    }
    
    if(args->p == UNSET) {
        fprintf(stderr, "Zadajte -p <program>\n");
        exit(EXIT_FAILURE);
    }
}
void printARGS(ARGS *args) {
    printf("program:     %s\n", (args->p)? "SET" : "UNSET");
    printf("programPath: %s\n", strlen(args->program)? args->program : NULL);
    printf("file:        %s\n", (args->f)? "SET" : "UNSET");
    printf("filePath:    %s\n", args->startDir);
}
int main(int argc, char *argv[]) {
    ARGS args;
    parseArguments(argc,argv,&args);
    validateArgs(&args);
//    while(1) {
        executeChoice(&args);
//        getPathFromChild(&args);
//    }
    return 0;
}
