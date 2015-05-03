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

	CHECK((msgQueueId = msgget(MSG_QUEUE_KEY, 0600)));

	CHECK(msgrcv(msgQueueId, &msgNumStudent, sizeof(msgNumStudent.numStudents), MSG_TYPE_NUM_STUDENTS, 0));

	int i;
	for(i = 0; i < msgNumStudent.numStudents; i++ ) {
		CHECK(msgrcv(msgQueueId, &msgStudent, sizeof(msgStudent.student), MSG_TYPE_STUDENT, 0));
		PrintStudent( &msgStudent.student);
	}
	
	CHECK(msgctl(msgQueueId, IPC_RMID, NULL));

	return EXIT_SUCCESS;
}

