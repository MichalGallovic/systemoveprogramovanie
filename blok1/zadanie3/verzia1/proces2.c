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

#define FIFO_PATH "/tmp/fifo_zad3_ref"

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

typedef enum{
    UNSET = 0,
    SET = 1
} OPTION;

typedef struct {
    char *startDir;
    pid_t parent_pid;
} ARGS;


/** 
* @brief Vynuluje argumenty
* 
* @param args
*/
void initArguments(ARGS *args) {
    args->startDir = NULL;
}

/** 
* @brief Hlavna funkcia, prehlada zadany priecinok
* sanitizne dirName ak neopsahuje trailing slash
* a vypise linky na zaklade nastaveni
* 
* @param args
*/
void listDir(char *startDir, pid_t parent_pid) {
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

    pid_t proc_pid = getpid();
    
    sigset_t set;
    int signum;
    sigaddset(&set, SIGUSR1);
    CHECK(sigprocmask(SIG_SETMASK, &set, NULL) != -1);
    
    if(getppid() == parent_pid) {
        CHECK(kill(parent_pid, SIGUSR2) != -1);   
    }
    while( (entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strncpy(file + dirLength, entry->d_name, sizeof(file) - dirLength);
            
            CHECK(lstat(file, &st) != -1);
            
            if(S_ISDIR(st.st_mode)) {
                pid_t pid = fork();
                if(pid == 0) {
                    listDir(file, parent_pid);
                    exit(EXIT_SUCCESS);
                } else {
                    wait(NULL);
                }
            }
            
            if (S_ISREG(st.st_mode)) {
                CHECK(sigwait(&set, &signum) == 0);
                fprintf(stderr, "proces: %d %s\n",(int)proc_pid, file);
                CHECK(kill(parent_pid, SIGUSR1) != -1);
            }
            
        }
    }

    closedir(dir);
}

void executeChoice(ARGS *args) {
    listDir(args->startDir, args->parent_pid);
}
void printARGS(ARGS *args) {
    printf("parentPid:    %d\n", args->parent_pid);
}
void parseArguments(int argc,char *argv[], ARGS *args) {
    initArguments(args);
    int opt;
    do {
        opt = getopt(argc, argv, "");

    } while(opt != -1);
    
    if (optind < argc) {
        args->startDir = argv[optind++];
        args->parent_pid = (pid_t) atoi(argv[optind]);
    }    

}
int main(int argc, char *argv[]) {
    ARGS args;
    parseArguments(argc, argv, &args);
    executeChoice(&args);
    return 0;
}
