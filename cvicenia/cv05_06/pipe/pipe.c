#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../common/student.h"

void runReader(int input, int output) {
	STUDENT student;


	//while(){
	//	PrintStudent(&student);
	//}
	printf("child: vsetko precitane\n");

	
}

void runWriter(int input, int output) {
	STUDENT student;
 

	SetStudent(& student, "meno1", 1);
	//poslanie udajov o studentovi
	SetStudent(& student, "meno2", 2);
	//poslanie udajov o studentovi

	  
}

int main(int argc, char * argv[]) {
	pid_t pid;
	int channel[2]; //komunikacny kanal implementovany nepomenovanou rurou

	//vytvorenie nepomenovanej rury
	

	pid = fork();
	switch(pid) {
	case -1:
		perror("fork");
		break;
	case 0:
		runReader(channel[12345 /*ZMENIT*/], channel[12345 /*ZMENIT*/]); //detsky process
		break;
	default:
		runWriter(channel[12345 /*ZMENIT*/], channel[12345 /*ZMENIT*/]); //rodicovsky process		
		wait(NULL);
	}

	return EXIT_SUCCESS;
}
