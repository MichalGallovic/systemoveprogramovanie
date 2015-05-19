#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void on_signalusr1(int sig)
{
    fprintf(stderr, "sigusr1 received!\n");
}


int main()
{
    struct sigaction sigact;
    memset(&sigact,0, sizeof(sigact));
    sigact.sa_handler = &on_signalusr1;

    //signal(SIGUSR1, &on_signalusr1);
    raise(SIGUSR1);//send normally
    
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK,&sigset,NULL);

    fprintf(stderr,"before calling SIGUSR1\n");
    raise(SIGUSR1); //nothing happens
    fprintf(stderr,"after calling SIGUSR1\n");
    
    int sig;
    int result = sigwait(&sigset,&sig);
    if (result == 0)
    {
        fprintf(stderr, "sigwait got signal: %d \n",sig);    
    }

    raise(SIGUSR1);//raised!
    raise(SIGUSR1);//nothing
    sigprocmask(SIG_UNBLOCK,&sigset,NULL);
    fprintf(stderr,"unblocking signal and raising signal \n");
    raise(SIGUSR1);//raised!
    fprintf(stderr,"unblocking signal \n");



    return 0;

}
