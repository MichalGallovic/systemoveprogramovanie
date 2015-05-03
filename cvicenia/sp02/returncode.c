#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int child() 
{
	srand(time(NULL));
	int returnCode = rand()%10;
	printf("child: returnCode = %d\n", returnCode);
	return returnCode;
}

void parent() {
	int status;
	wait(&status);
	if(	WIFEXITED(status))
		printf("parent: child returnCode = %d\n", WEXITSTATUS(status));
	else
		printf("parent: child terminated abnormally");
}

int main() {
	pid_t pid;
	int childReturnCode;

	pid = fork();
	if( pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if( pid == 0 ) {
		childReturnCode = child();
		exit(childReturnCode);
	}
	else {
		parent();
		exit(EXIT_SUCCESS);
	}
}
	
