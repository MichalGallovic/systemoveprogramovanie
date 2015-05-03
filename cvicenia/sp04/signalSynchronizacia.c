#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int childProcess() {
	//sleep(2); //simulacia oneskorenia, aby sme overili co sa stane ak rodic posle signal skor, ako dieta zavola sigwait
	sigset_t set;
	int sig_num;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	printf("child: pred cakanim na signal\n");
	if(sigwait(&set, &sig_num) != 0){
		perror("sigwait");
		return EXIT_FAILURE;
	}
	printf("child: za cakanim na signal\n");

	return EXIT_SUCCESS;
}

int parentProcess(pid_t childPid) {
	sleep(1);  //simulacia oneskorenia, aby sme overili ci detsky proces pocka na signal
	printf("parent: posielanie signalu SIGUSR1\n");
	if(kill(childPid, SIGUSR1) != 0 ){
		perror("kill");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	pid_t pid; //navratova hodnota fork
	sigset_t set;
	
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	if(sigprocmask(/*SIG_BLOCK*/ SIG_SETMASK, &set, NULL) != 0){
		perror("sigprocmask");
		return EXIT_FAILURE;
	}
	
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
