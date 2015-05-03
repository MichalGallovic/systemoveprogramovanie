#include "commonMsgQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/msg.h>

int main(int argc, char * argv[]) 
{
	int msgQueueId; 
	MSG_NUM_STUDENTS msgNumStudent; 
	MSG_STUDENT msgStudent; 

	CHECK((msgQueueId = msgget(MSG_QUEUE_KEY, IPC_CREAT | IPC_EXCL | 0600))); 

	msgNumStudent.mtype = MSG_TYPE_NUM_STUDENTS;
	msgNumStudent.numStudents = 2;

	CHECK(msgsnd(msgQueueId, &msgNumStudent, sizeof(msgNumStudent.numStudents), 0));

	msgStudent.mtype = MSG_TYPE_STUDENT;
	SetStudent(& msgStudent.student, "Meno1", 1);
	CHECK(msgsnd(msgQueueId, &msgStudent, sizeof(msgStudent.student), 0));	

	SetStudent(& msgStudent.student, "Meno2", 2);
	CHECK(msgsnd(msgQueueId, &msgStudent, sizeof(msgStudent.student), 0));	

	return EXIT_SUCCESS;
}

