#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

//doplnte funkciu SignalHandler
void SignalHandler(int signal) {
    printf("SignalHandler process ID: %d\n", signal);
}
int main()
{
    struct sigaction sa_int;
    memset(&sa_int,0,sizeof(sa_int));
    sa_int.sa_handler = &SignalHandler;
    sigaction(SIGINT,&sa,NULL);
	
    while(1) {
		printf(".");
		fflush(stdout);
		sleep(1);
	}
	
	return EXIT_SUCCESS;
}

 
