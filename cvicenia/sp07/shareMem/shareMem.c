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
// - precita a vypise ulohu na standardny vystup (udaje zo zdielanej pamate zapisane ucitelom)
// - vypracuje ulohu (doplni udaje v zdielanej pamati) a vypise vypracovanu ulohu na standardny vystup
// - oznami ucitelovi, ze je uloha vypracovana (pouzije semafor)
//Vstupy: shmId - id zdielanej pamate
//        semId - id mnoziny semaforov
void RunStudentProcess(int shmId, int semId)
{
	Homework * homework; //adresa zaciatku zdielanej pamate (so zdielanou pamatou budeme pracovat ako so strukturou)
	struct sembuf  * sops;

	CHECK((homework = (Homework *)shmat(shmId, NULL, 0)) != (Homework *) -1); 

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

	CHECK(shmdt((void *)homework) != -1);	
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
	Homework * homework; 
	struct sembuf * sops;
	
	CHECK((homework = (Homework *)shmat(shmId, NULL, 0)) != (Homework *) -1); 
	
	//ucitel vygenerovanie novu ulohu pre studenta
	GenerateRandomHomework(homework);

	//vypis zadanej ulohy
	PrintHomework(homework);

	sleep(5); 
	
	sops->sem_num = SEM_HW_ASSIGNED;
	sops->sem_op = +1;
	sops->sem_flg = 0;

	CHECK(semop(semId, sops, 1) != -1);

	sops->sem_num = SEM_HW_COMPLETE;
	sops->sem_op = -1;
	CHECK(semop(semId, sops, 1) != -1);

	//vypis vypracovanej ulohy
	PrintHomework(homework);

	CHECK(shmdt((void *)homework) != -1);	
}

int main()
{
	int shmId; //id zdielanej pamate
	int semId; //id mnoziny semaforov
	pid_t pid; //pomocna premenna pre PID procesov
	union semun sem_un;
	unsigned short sem_val[SEM_COUNT];

	CHECK((shmId = shmget(IPC_PRIVATE, sizeof(Homework), 0660)) != -1);

	CHECK((semId = semget(IPC_PRIVATE, SEM_COUNT, IPC_CREAT | 0600)) != -1);
 	
	sem_val[SEM_HW_ASSIGNED] = 0;
	sem_val[SEM_HW_COMPLETE] = 0;
	sem_un.array = sem_val;
	CHECK(semctl(semId, 0, SETALL, sem_un) != -1);

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

	CHECK( wait(NULL) != -1);
	CHECK( wait(NULL) != -1);

	CHECK(shmctl(shmId, IPC_RMID, 0) != -1);
	
	CHECK(semctl(semId, 0, IPC_RMID) != -1);
	
	return EXIT_SUCCESS;
}
