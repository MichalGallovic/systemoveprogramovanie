#ifndef STUDENT_H
#define STUDENT_H

//maximalna dlzka mena studenta (vratane '\0')
#define STUDENT_NAME_SIZE 100

//struktura reprezentujuca informacie o studentovi
typedef struct {
	char name[STUDENT_NAME_SIZE];
	int  id; //osobne cislo
} STUDENT;

//nastavenie poloziek struktury
void SetStudent(STUDENT * student, char * name, int id);

//vypis informacii o studentovi na standardny vystup
void PrintStudent(STUDENT * student);

#endif //STUDENT_H

