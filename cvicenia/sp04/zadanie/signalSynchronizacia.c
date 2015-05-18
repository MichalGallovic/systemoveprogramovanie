#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int childProcess() {
	//sleep(2); //simulacia oneskorenia, aby sme overili co sa stane ak rodic posle signal skor, ako dieta zavola sigwait

	printf("child: pred cakanim na signal\n");
	//tu volajte sigwait
	printf("child: za cakanim na signal\n");

	return EXIT_SUCCESS;
}

int parentProcess(pid_t childPid) {
	sleep(1);  //simulacia oneskorenia, aby sme overili ci detsky proces pocka na signal
	printf("parent: posielanie signalu SIGUSR1\n");
	//tu poslite signal SIGUSR1
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	pid_t pid; //navratova hodnota fork


	pid = fork();
	if( pid == -1 ) {
		perror("fork");
		exit( EXIT_FAILURE);
	}
	else if( pid == 0 ) { //detsky prosec
		return childProcess();
	}
	else { //rodicovsky proces
		return parentProcess(pid);
	}
}
