/*
Komunikacia cez mapovanu pamat

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

#include <sys/mman.h>
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
// - precita a vypise ulohu na standardny vystup (udaje z mapovaneho suboru zapisane ucitelom)
// - vypracuje ulohu (doplni udaje v mapovanom subore) a vypise vypracovanu ulohu na standardny vystup
// - oznami ucitelovi, ze je uloha vypracovana (pouzije semafor)
//Vstupy: fileDescr - deskriptor suboru, ktory sa bude mapovat do pamate
//        semId     - id mnoziny semaforov
void RunStudentProcess(int fileDescr, int semId)
{
	Homework * homework; //adresa zaciatku mapovanej pamate (s mapovanou pamatou budeme pracovat ako so strukturou)

	//namapovanie suboru do pamate
	//TODO homework =
	
	//zatvorenie suboru
	//TODO

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
		
	//odmapovanie suboru z pamate
	//TODO
}

//Funkcia ucitela, ktoru bude vykonavat detsky proces
//Proces bude postupovat nasledovne:
// - zada ulohu (zapise ulohu do mapovaneho suboru) a vypise ju na stdandardny vystup 
// - oznami studentovi, ze uloha je kompletne zadana
// - pocka na vypracovanie ulohy studentom
// - vypise vypracovanu ulohu na standardny vystup
//Vstupy: fileDescr - deskriptor suboru, ktory sa bude mapovat do pamate
//        semId     - id mnoziny semaforov
void RunTeacherProcess(int fileDescr, int semId)
{
	Homework * homework; //adresa zaciatku mapovanej pamate (s mapovanou pamatou budeme pracovat ako so strukturou)

	//namapovanie suboru do pamate
	//TODO homework =
	
	//zatvorenie suboru
	//TODO
	
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

	//odmapovanie suboru z pamate
	//TODO
}

int main()
{
	int fileDescr; //deskriptor suboru ktory bude mapovany
	int semId; //id mnoziny semaforov
	pid_t pid; //pomocna premenna pre PID procesov
	
	//vytvorenie a otvorenie suboru s jedinecnym nazvom
	//TODO fileDescr = mkstemp(parameter)
	
	//zvacsenie dlzky suboru
	//TODO
	
	//vytvorenie mnoziny dvoch semaforov a ziskanie id mnoziny semaforov
	//TODO semId =

	//inicializacia semaforov na nulove hodnoty
	//TODO
				 
	//vytvorenie procesov ucitela a studenta
	CHECK( (pid = fork()) != -1);
	if( pid == 0 ) {
		RunStudentProcess(fileDescr, semId);
		exit(EXIT_SUCCESS);
	}
	CHECK( (pid = fork()) != -1);
	if( pid == 0 ) {
		RunTeacherProcess(fileDescr, semId);
		exit(EXIT_SUCCESS);
	}

	//zatvorenie suboru a jeho odstrananie zo suboroveho systemu 
	//(detske procesy mozu stale so suborom pracovat, pretoze ho maju otvoreny)
	//TODO
	
	//cakanie na ukoncenie procesov ucitela a studenta
	CHECK( wait(NULL) != -1);
	CHECK( wait(NULL) != -1);

	//odstranenie mnoziny semaforov
	//TODO 
	
	return EXIT_SUCCESS;
}
