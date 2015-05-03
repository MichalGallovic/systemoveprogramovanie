#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void child(int childId) {
	sleep(1);
	printf("child %d\n", childId);
}

int main()
{
	const int NUM_CHILDREN = 22;
	//pid_t pid;
	int childId;

	for(childId = 1; childId <= NUM_CHILDREN; childId++) {
		switch(fork()){
			case -1:
				perror("fork");
				return(EXIT_FAILURE);
			case 0:
				child(childId);
				return(EXIT_SUCCESS);	
			default:
				break;
		}
	}

	for(childId = 1; childId <= NUM_CHILDREN; childId++) {
		wait(NULL);	
	}
	printf("detske procesy ukoncene\n");

	return EXIT_SUCCESS;
}

