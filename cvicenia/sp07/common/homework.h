#ifndef HOMEWORK_H
#define HOMEWORK_H

//uloha na vypocet sumy cisiel
typedef struct {
	int augend; //1. scitanec
	int addend; //2. scitanec
	int sum;    //sucet
} Homework;

//vytlaci ulohu na standardny vystup ako textovy retazec
void PrintHomework(Homework * homework);

//vygeneruje (vyplni) ulohu novym zadanim (nastavi prislusne polozky struktury)
void GenerateRandomHomework(Homework * homework);

//vypracovanie ulohy (vypocita sumu scitancov a nastavi prislusnu polozku struktury)
void DoHomework(Homework * homework);

#endif //HOMEWORK_H