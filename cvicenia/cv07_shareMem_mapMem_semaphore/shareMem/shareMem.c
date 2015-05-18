/*
Komunikacia cez zdielanu pamat

Pseudokod synchronizacie

        rodic:
             inicializuj(SEM_HW_ASSIGNED , 0) //semafor pre synchronizaciu zadania ulohy    
             inicializuj(SEM_HW_COMPLETE,  0) //semafor pre synchronizaciu odovzdania ulohy 
    
Teacher (dieta):            Student (dieta):
    zadaj_ulohu                 wait(SEM_HW_ASSIGNED)
    post(SEM_HW_ASSIGNED)       vypracuj_ulohu
    wait(SEM_HW_COMPLETE)       post(SEM_HW_COMPLETE)
    vypis_vypracovanu_ulohu     
*/

#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include "../common/utility.h"
#include "../common/homework.h"

//konstanty pre oznacenie semaforov
enum semaphores {
	SEM_HW_ASSIGNED,  //semafor pre synchronizaciu zadania domacej ulohy
	SEM_HW_COMPLETE,  //semafor pre synchronizaciu odovzdania domacej ulohy
	SEM_COUNT         //pocet semaforov
};

//union pre pracu so semaformi (definovany podla manualu)
//TODO (man semctl)




//Funkcia studenta, ktoru bude vykonavat detsky proces 
//Proces bude postupovat nasledovne: 
// - pocka na zadanie ulohy ucitelom (pouzije semafor)
// - precita a vypise ulohu na standardny vystup (udaje zo zdielanej pamate zapisane ucitelom)
// - vypracuje ulohu (doplni udaje v zdielanej pamati) a vypise vypracovanu ulohu na standardny vystup
// - oznami ucitelovi, ze je uloha vypracovana (pouzije semafor)
//Vstupy: shmId - id zdielanej pamate
//        semId - id mnoziny semaforov
void RunStudentProcess(int shmId, int semId)
{
	Homework * homework; //adresa zaciatku zdielanej pamate (so zdielanou pamatou budeme pracovat ako so strukturou)

	//pripojenie zdielanej pamate
	//TODO homework = 

	//cakanie na zadanie ulohy (vykonanie operacie wait na semafore s cislom SEM_HW_ASSIGNED)
	//TODO

	//vypis zadanej ulohy pred vypracovanim
	PrintHomework(homework);
	
	//vypracovanie ulohy
	DoHomework(homework);

	//vypis vypracovanej ulohy
	PrintHomework(homework);

	//sleep(5); //testovanie ci ucitel caka na oznamenie o vypracovani ulohy
	
	//oznamenie o vypracovani ulohy (operacia signal (post) na semafore s cislom SEM_HW_COMPLETE) 
	//TODO
	
	//odpojenie zdielanej pamate
	//TODO
}

//Funkcia ucitela, ktoru bude vykonavat detsky proces
//Proces bude postupovat nasledovne:
// - zada ulohu (zapise ulohu do zdielanej pamate) a vypise ju na standardny vystup 
// - oznami studentovi, ze uloha je kompletne zadana
// - pocka na vypracovanie ulohy studentom
// - vypise vypracovanu ulohu na standardny vystup
//Vstupy: shmId - id zdielanej pamate
//        semId - id mnoziny semaforov
void RunTeacherProcess(int shmId, int semId)
{
	Homework * homework; //adresa zaciatku zdielanej pamate (so zdielanou pamatou budeme pracovat ako so strukturou)

	//pripojenie zdielanej pamate
	//TODO homework = 
	
	//ucitel vygenerovanie novu ulohu pre studenta
	GenerateRandomHomework(homework);

	//vypis zadanej ulohy
	PrintHomework(homework);

	//sleep(5); //testovanie ci student caka na oznamenie o zadani ulohy

	//oznamenie o zadani ulohy (operacia signal (post) nad semaforom s cislom SEM_HW_ASSIGNED)
	//TODO

	//cakanie na vypracovanie ulohy (operacia wait nad semaforom s cislom SEM_HW_COMPLETE)
	//TODO

	//vypis vypracovanej ulohy
	PrintHomework(homework);

	//odpojenie zdielanej pamate
	//TODO
}

int main()
{
	int shmId; //id zdielanej pamate
	int semId; //id mnoziny semaforov
	pid_t pid; //pomocna premenna pre PID procesov
	
	//vytvorenie zdielanej pamate a ziskanie jej id
	//TODO shmId = 

	//vytvorenie mnoziny dvoch semaforov a ziskanie id mnoziny semaforov
	//TODO semId = 

	//inicializacia semaforov na nulove hodnoty
	//TODO

	//vytvorenie procesov ucitela a studenta
	CHECK( (pid = fork()) != -1);
	if( pid == 0 ) {
		RunStudentProcess(shmId, semId);
		exit(EXIT_SUCCESS);
	}
	CHECK( (pid = fork()) != -1);
	if( pid == 0 ) {
		RunTeacherProcess(shmId, semId);
		exit(EXIT_SUCCESS);
	}

	//cakanie na ukoncenie procesov ucitela a studenta
	CHECK( wait(NULL) != -1);
	CHECK( wait(NULL) != -1);

	//oznacenie zdielanej pamate na odstranenie zo systemu
	//TODO

	//odstranenie mnoziny semaforov
	//TODO 
	
	return EXIT_SUCCESS;
}
