// BASIC KOMUNIKACIA PIPA 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


void runReader(int input, int output) {
    char pismeno;

    close(output);

    while( read(input, & pismeno, sizeof(pismeno)) == sizeof(char) ) {
        printf("%c\n", pismeno);
    }
    printf("child: vsetko precitane\n");

    close(input);
}

void runWriter(int input, int output) {
    close(input);
    char * anakonda = "anakonda";

    write(output, &anakonda[0], sizeof(char));
    write(output, &anakonda[1], sizeof(char));
    write(output, &anakonda[2], sizeof(char));
    write(output, &anakonda[3], sizeof(char));
    write(output, &anakonda[4], sizeof(char));
    write(output, &anakonda[5], sizeof(char));
    write(output, &anakonda[6], sizeof(char));
    write(output, &anakonda[7], sizeof(char));

    close(output);
}

int main() {
    pid_t pid;
    int channel[2];

    //vytvorenie nepomenovanej rury
    if( pipe(channel) == -1 ) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    switch(pid) {
        case -1:
            perror("fork");
            break;
        case 0:
            runReader(channel[0], channel[1]); //detsky process
            break;
        default:
            runWriter(channel[0], channel[1]); //rodicovsky process
            wait(NULL);
    }

    return EXIT_SUCCESS;
}