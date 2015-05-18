#include "student.h"
#include <string.h>
#include <stdio.h>

void SetStudent(STUDENT * student, char * name, int id)
{
	strncpy(student->name, name, STUDENT_NAME_SIZE);
	student->name[STUDENT_NAME_SIZE-1] = '\0';
	student->id = id;
}

void PrintStudent(STUDENT * student)
{
	printf("meno: %s, id: %d\n", student->name, student->id);
}

