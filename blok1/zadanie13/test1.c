#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void main() {
    int fds[2];
    int status;
    pipe(fds);
    if (fork() == 0) {
        close(0);
        dup(fds[0]); // redirect standard input to fds[0]
        close(fds[1]); // close unused end of pipe
        if (execlp("wc","wc","-l",NULL))
        {
            printf("wc failed\n");
        }
        exit(0);
    } if (fork() == 0) {
        close(1);
        dup(fds[1]);     // redirect standard output to fds[1];
        close(fds[0]);   // close unused end of pipe
        if(execlp("find","find",".","-type","d","-not","-empty",NULL))
        {
            printf("find failed\n");
        }
        exit(0);
    }
    close(0);
    close(1);           // parent
    waitpid(-1, &status, 0);
    // parent is waiting for both children
}
