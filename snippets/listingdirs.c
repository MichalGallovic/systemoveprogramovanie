// VYPIS ADRESARA A ADRESARA REKURZIVNE 

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

void listDir(char *nazovPriecinka) {
    DIR *dir;
    struct dirent *entry;
    char *dirName;
    size_t dirLength = strlen(nazovPriecinka);
    dirName = (char*) malloc(dirLength*sizeof(char));
    strcpy(dirName, nazovPriecinka);

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

    char file[PATH_MAX + 1];
    strcpy(file, dirName);

    while( (entry = readdir(dir)) != NULL) {
        strncpy(file + dirLength, entry->d_name, sizeof(file) - dirLength);
        printf("%s\n", file);
    }

    closedir(dir);
}

void listDirRecursive(char *startDir) {
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

    while( (entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strncpy(file + dirLength, entry->d_name, sizeof(file) - dirLength);

            CHECK(lstat(file, &st) != -1);

            if(S_ISDIR(st.st_mode)) {
                pid_t childPid = fork();
                if(childPid == 0) {
                    listDirRecursive(file);
                    exit(EXIT_SUCCESS);
                } else {
                    wait(NULL);
                }
            }

            if (S_ISREG(st.st_mode)) {
                fprintf(stderr, "%s\n",file);
            }

        }
    }

    closedir(dir);
}
int main() {
    listDir(".");
    listDirRecursive("..");

    return EXIT_SUCCESS;
}