#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

//doplnte funkciu SignalHandler

int main()
{
	//vypis PID
	printf("PID = %d\n", getpid());

	//nastavenie signal handlera

	
	//synchronne cakanie na signal
	
	printf("pred sigsuspend\n");
	//doplnte volanie funkcie sigsuspend
	printf("za sigsuspend\n");
	
	return EXIT_SUCCESS;
}

 
