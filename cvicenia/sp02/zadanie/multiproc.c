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
	const int NUM_CHILDREN = 5;
	pid_t pid;
	int childId;

	//vytvorenie detskych procesov
	for(childId = 1; childId <= NUM_CHILDREN; childId++) {
			//child(childId);
	}

	//pockanie na ukoncenie detskych procesov
	for(childId = 1; childId <= NUM_CHILDREN; childId++) {
		
	}
	printf("detske procesy ukoncene\n");

	return EXIT_SUCCESS;
}

