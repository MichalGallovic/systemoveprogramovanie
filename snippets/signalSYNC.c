#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


int main()
{

    sigset_t sigset;
    sigemptyset( &sigset);
    sigaddset( &sigset, SIGINT);
    sigprocmask(SIG_SETMASK, &sigset, NULL);
    int signum;
    printf("pred sigsuspend\n");
    sigwait( &sigset, &signum);
    printf("za sigsuspend %d\n", signum);

    return EXIT_SUCCESS;
}