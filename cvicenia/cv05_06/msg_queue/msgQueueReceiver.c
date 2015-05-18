#include "commonMsgQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/msg.h>

int main(int argc, char * argv[]) 
{
	int msgQueueId; //id fornty sprav
	MSG_NUM_STUDENTS msgNumStudent; //sprava pre prijatie udaju o pocte studentov
	MSG_STUDENT msgStudent; //sprava pre prijatie udajov o studentovi

	//podla kluca ziskajte id fronty sprav 

	//nacitanie poctu studentov

	//nacitanie informaci o studentoch 
	int i;
	for(i = 0; i < msgNumStudent.numStudents; i++ ) {
	
		
		//vypis prijatej spravy
		PrintStudent( &msgStudent.student);
	}
	
	//zrusenie fronty sprav

	return EXIT_SUCCESS;
}

