#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void SignalHandler(int signal_num){
	printf("\nsignal number: %i\n", signal_num);
}

int main(){

	printf("pid: %i\n", getpid());
	
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	
	sa.sa_handler = &SignalHandler;
	if (sigaction(SIGUSR1, &sa, NULL) !=  0){
		perror("sigaction");
		return EXIT_FAILURE;
	}

        if (sigaction(SIGINT, &sa, NULL) !=  0){
                perror("sigaction");
                return EXIT_FAILURE;
        }

	while(1) {
		printf(".");
		fflush(stdout);
		sleep(1);
	}
	
	return EXIT_SUCCESS;
}
