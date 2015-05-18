#ifndef COMMON_MSG_QUEUE_H
#define COMMON_MSG_QUEUE_H

#include "../common/student.h"
#include "../common/utility.h"

#define MSG_QUEUE_KEY 12345     //kluc identifikujuci frontu sprav 
#define MSG_TYPE_NUM_STUDENTS 1 //typ spravy pre poslanie poctu studentov
#define MSG_TYPE_STUDENT 2      //typ spravy pre poslanie informacii o studentovi

//struktura pre spravu obsahujucu pocet studentov (definovat podla manualu)
typedef struct {
	long mtype;       /* message type, must be > 0 */
	int numStudents;  /* message data */                //pocet studentov
} MSG_NUM_STUDENTS;

//struktura pre spravu obsahujucu informacie o studentovi (definovat podla manualu)
typedef struct {
	long mtype;       /* message type, must be > 0 */
	STUDENT student;  /* message data */               //informacie o studentovi
} MSG_STUDENT;

#endif //COMMON_MSG_QUEUE_H

