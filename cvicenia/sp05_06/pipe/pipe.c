#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../common/student.h"
#include "../common/utility.h"

void runReader(int input, int output) {
	STUDENT student;

	CHECK(close(output));
	while(read(input, &student, sizeof(STUDENT)) == sizeof(STUDENT)){
		PrintStudent(&student);
	}
	printf("child: vsetko precitane\n");

	CHECK(close(input));
}

void runWriter(int input, int output) {
	STUDENT student;
 
	CHECK(close(input));

	SetStudent(& student, "meno1", 1);
	write(output, &student, sizeof(STUDENT)); 

	SetStudent(& student, "meno2", 2);
	write(output, &student, sizeof(STUDENT)); 
	
	CHECK(close(output));	  
}

int main(int argc, char * argv[]) {
	pid_t pid;
	int channel[2]; 
	
	CHECK(pipe(channel));
	

	pid = fork();
	switch(pid) {
	case -1:
		perror("fork");
		break;
	case 0:
		runReader(channel[0], channel[1]); 
		break;
	default:
		runWriter(channel[0], channel[1]); 		
		wait(NULL);
	}

	return EXIT_SUCCESS;
}
