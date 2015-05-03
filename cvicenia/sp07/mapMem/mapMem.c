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

union semun {
	int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};




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
	struct sembuf  * sops;

	CHECK((homework = (Homework *)mmap(NULL, sizeof(Homework), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescr, 0)) != (Homework *) -1);
	
	CHECK(close(fileDescr) != -1);

	sops->sem_num = SEM_HW_ASSIGNED;
	sops->sem_op = -1;
	sops->sem_flg = 0;

	CHECK(semop(semId, sops, 1) != -1);

	//vypis zadanej ulohy pred vypracovanim
	PrintHomework(homework);
	
	//vypracovanie ulohy
	DoHomework(homework);

	//vypis vypracovanej ulohy
	PrintHomework(homework);

	sleep(5); //testovanie ci ucitel caka na oznamenie o vypracovani ulohy
	
	sops->sem_num = SEM_HW_COMPLETE;
	sops->sem_op = +1;
	CHECK(semop(semId, sops, 1) != -1);
		
	CHECK(munmap((void *) homework, sizeof(Homework)) != -1);
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
	struct sembuf * sops;

	CHECK((homework = (Homework *)mmap(NULL, sizeof(Homework), PROT_READ | PROT_WRITE, MAP_SHARED, fileDescr, 0)) != (Homework *) -1);
	
	CHECK(close(fileDescr) != -1);

	//ucitel vygenerovanie novu ulohu pre studenta
	GenerateRandomHomework(homework);

	//vypis zadanej ulohy
	PrintHomework(homework);

	sleep(5); //testovanie ci student caka na oznamenie o zadani ulohy

	sops->sem_num = SEM_HW_ASSIGNED;
	sops->sem_op = +1;
	sops->sem_flg = 0;

	CHECK(semop(semId, sops, 1) != -1);
	
	sops->sem_num = SEM_HW_COMPLETE;
	sops->sem_op = -1;
	CHECK(semop(semId, sops, 1) != -1);

	//vypis vypracovanej ulohy
	PrintHomework(homework);

	CHECK(munmap((void *) homework, sizeof(Homework)) != -1);
}

int main()
{
	int fileDescr; //deskriptor suboru ktory bude mapovany
	int semId; //id mnoziny semaforov
	pid_t pid; //pomocna premenna pre PID procesov
	union semun sem_un;
	unsigned short sem_val[SEM_COUNT];
	char file_name[] = "/tmp/spXXXXXX";

	CHECK((fileDescr = mkstemp(file_name)) != -1);

	CHECK(lseek(fileDescr, sizeof(Homework) - 1, SEEK_SET) != -1);
	CHECK(write(fileDescr, "", 1) != -1 );
	
	CHECK((semId = semget(IPC_PRIVATE, SEM_COUNT, IPC_CREAT | 0600)) != -1);
 	
	sem_val[SEM_HW_ASSIGNED] = 0;
	sem_val[SEM_HW_COMPLETE] = 0;
	sem_un.array = sem_val;
	CHECK(semctl(semId, 0, SETALL, sem_un) != -1);

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

	CHECK(close(fileDescr) != -1);
	CHECK(unlink(file_name) != -1);
	
	//cakanie na ukoncenie procesov ucitela a studenta
	CHECK( wait(NULL) != -1);
	CHECK( wait(NULL) != -1);

	CHECK(semctl(semId, 0, IPC_RMID) != -1);

	return EXIT_SUCCESS;
}
