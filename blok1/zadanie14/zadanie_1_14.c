/** 
* @brief Zadanie
* Napiste program, ktory v adresarovej strukture, urcenej prvym argumentom programu <pociatocy_priecinok>, najde vsetky
* symbolicke linky a vypise ich zoznam na standardny vystup. Ak bude program spusteny s prepinacom "-e"/"--exist", vyhlada len
* pocet liniek s existujucim cielovym suborom; s prepinacom "-n" alebo "--not_exist" len pocet liniek s neexistujucim cielovym
* suborom. Po spracovani pouzivatelom zadanej adresarovej struktury ma mat pouzivatel moznost pokracovat v spracovani inej
* adresarovej struktury. Ak sa pouzivatel rozhodne pokracovat v cinnosti progeamu, vytvorte detsky proces, ktory od pouzivatela
* vypyta nazov dalsieho pociatocneho adresara. Rodicovsky proces nech caka na spracovanie vstupu od pouzivatela detskym
* procesom. Po spracovani vstupu od pouzivatela da detsky proces rodicovskemu pomocou signalu vediet, ze moze pokracovat vo
* vykonavani cinnosti. Program musi pouzivat volania opendir, readdir a closedir.
* Volanie programu: zadanie.elf <pociatocny_priecinok> [-e|--exist | -n | --not_exist]
*/

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

#define FIFO_PATH "/tmp/fiforef"

typedef enum{
    UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
    char startDir[PATH_MAX];
    int e;
    int n;
} ARGS;

void printHelpAndExit(FILE* stream, int exitCode){
    fprintf(stream, "Usage: parametre [-e | --exist] [-n | --not_exist] [<dir>]\n");
    fprintf(stream, "Program vypise symbolicke linky v adresarovej strukture. Ak su definovane prepinace, zobrazia sa iba tie, ktore ukazuju na existujuci / neexistujuci subor\n");
    exit(exitCode);
}

/** 
* @brief Vynuluje argumenty
* 
* @param args
*/
void initArguments(ARGS *args) {
    args->e = UNSET;
    args->n = UNSET;
    args->startDir[0] = '\0';
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
        strncpy(args->startDir, argv[optind], sizeof(args->startDir));
        break;
    }

}

/** 
* @brief Validacia argumentov
* Error ak nezadame pociatoc priecinok
* alebo ak je zadane -e aj -n naraz
* 
* @param args
*/
void validateArgs(ARGS *args){
    if(strlen(args->startDir) == 0) {
        fprintf(stderr,"Zadajte pociatocny priecinok\n");
        printHelpAndExit(stderr, EXIT_FAILURE);
    }
    if(args->e == SET && args->n == SET) {
        fprintf(stderr,"Vyberte si iba jednu volbu z -e alebo -n\n");
        printHelpAndExit(stderr, EXIT_FAILURE);
    }
}

/** 
* @brief Vypis liniek na zaklade nastavenych parametrov
* stat funkcia vracia -1 ak target file neexistuje
* @param path
* @param args
*/
void printLinkResult(char *path, ARGS *args) {
    struct stat buf;
    struct stat actualFile;

    if(lstat(path,&buf) == -1) {
        perror("Lstat");
        exit(EXIT_FAILURE);
    }
    if(S_ISLNK(buf.st_mode)) {
        if(args->e) {
            if(stat(path, &actualFile) != -1) {
                printf("%s\n", path);
            }
        } else if(args->n) {
            if(stat(path,&actualFile) == -1) {
                printf("%s\n", path);
            }
        } else {
            printf("%s\n", path);
        }
    }
    
}

/** 
* @brief Hlavna funkcia, prehlada zadany priecinok
* sanitizne dirName ak neopsahuje trailing slash
* a vypise linky na zaklade nastaveni
* 
* @param args
*/
void listDir(ARGS *args) {
    DIR *dir;
    struct dirent *entry;
    char *dirName;
    size_t dirLength = strlen(args->startDir);
    dirName = (char*) malloc(dirLength*sizeof(char));
    strcpy(dirName, args->startDir);

    if(dirName[dirLength - 1] != '/') {
        dirName[dirLength] = '/';
        dirName[dirLength + 1] = '\0';
        dirLength++;
    }

    dir = opendir(dirName);
    if(dir == NULL) {
        fprintf(stderr,"%s %s\n",strerror(errno), dirName);        
        return;
    }

    while( (entry = readdir(dir)) != NULL) {
        
        int entryPathLength = dirLength + strlen(entry->d_name) + 2;
        char *entryPath = (char *) malloc(entryPathLength * sizeof(char));
        sprintf(entryPath,"%s%s",dirName,entry->d_name);
        
        printLinkResult(entryPath, args); 
        
        free(entryPath);
    }

    closedir(dir);
}

void executeChoice(ARGS *args) {
    if(args->e) {
        printf("Vypisujem linky, ktore maju cielovy subor: \n");
    } else if(args->n) {
        printf("Vypisujem linky, ktore maju neexist. ciel. subor: \n");
    } else {
        printf("Vypisujem vsety linky:\n");
    }
    listDir(args);
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
    
    if(kill(getppid(), SIGUSR1) != 0) {
        perror("kill");
        exit(EXIT_FAILURE);
    }
    
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

    
    sigset_t set;
    int signum;
    sigaddset(&set, SIGUSR1);
    if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    
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
            if (sigwait(&set, &signum) != 0) {
                perror("sigwait");
                exit(EXIT_FAILURE);
            }
            break;
    }
    parent(args); 
}
int main(int argc, char *argv[]) {
    ARGS args;
    parseArguments(argc,argv,&args);
    validateArgs(&args);

    while(1) {
        executeChoice(&args);
        getPathFromChild(&args);
    }
    return 0;
}
