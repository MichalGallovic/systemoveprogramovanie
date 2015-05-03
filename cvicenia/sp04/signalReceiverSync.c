#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void SignalHandler(int signal_num){
	printf("\nSignal no: %i\n", signal_num);
}

int main()
{
	printf("PID = %d\n", getpid());
	
	sigset_t mask;
	struct sigaction sa;
	
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &SignalHandler;

	if(sigaction(SIGUSR1, &sa, NULL) != 0){
		perror("sigaction");
		return EXIT_FAILURE;
	}
	
        if(sigaction(SIGINT, &sa, NULL) != 0){
                perror("sigaction");
                return EXIT_FAILURE;
        }

	sigfillset(&mask);
    	sigdelset(&mask, SIGUSR1);

	printf("pred sigsuspend\n");
	sigsuspend(&mask);
	printf("za sigsuspend\n");
	
	return EXIT_SUCCESS;
}

 
