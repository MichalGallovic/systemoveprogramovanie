#include "commonMsgQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/msg.h>

int main(int argc, char * argv[]) 
{
	int msgQueueId; //id fronty sprav
	MSG_NUM_STUDENTS msgNumStudent; //sprava pre poslanie poctu studentov
	MSG_STUDENT msgStudent; //sprava pre poslanie udajov o studentovi

	//ziskanie id ku fronte fronte sprav

	//poslanie poctu studentov
	msgNumStudent.mtype = MSG_TYPE_NUM_STUDENTS;
	msgNumStudent.numStudents = 2;

	//poslanie informacie o studnetnoch
	//msgStudent.mtype = 
	SetStudent(& msgStudent.student, "Meno1", 1);
	//poslanie udajov 	

	SetStudent(& msgStudent.student, "Meno2", 2);
	//poslanie udajov

	return EXIT_SUCCESS;
}

