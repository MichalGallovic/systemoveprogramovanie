#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

static int* buf;

static int shmId; //id zdielanej pamate
static int semId; //id mnoziny semaforov
static int sem_num;
static int num_processes;

union semun {
		int val;    		 /* Value for SETVAL */
    	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    	unsigned short  *array;  /* Array for GETALL, SETALL */
    	struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

void deleteShared(){ CHECK(shmctl(shmId, IPC_RMID, 0) != -1);} 

void deleteSem(){ CHECK(semctl(semId, 0, IPC_RMID) != -1);}

void createShared(){
	CHECK((shmId = shmget(IPC_PRIVATE, getpagesize(), 0660)) != -1);
}

void createSem(int number){
	int i = 0;
	union semun sem_un;
	unsigned short *sem_val;
	sem_val = (unsigned short *) malloc(number * sizeof(unsigned short)); 	

	CHECK((semId = semget(IPC_PRIVATE, number, IPC_CREAT | 0600)) != -1);
	for(i = 0; i < number; ++i)  
        	sem_val[i] = 0;
        sem_un.array = sem_val;
        CHECK(semctl(semId, 0, SETALL, sem_un) != -1);
	free(sem_val);
}

void write_(int* buffer, size_t size){
	memcpy(buf, buffer, size);
}

void read_(int* buffer){
	memcpy(buffer, buf, sizeof(int));
}

void teardown(){
	CHECK(shmdt((void*)buf) != -1); 
	deleteShared();
	deleteSem();
}

void lockSem(int sem_num){
	struct sembuf sops;
	sops.sem_num = (unsigned short)sem_num; 
	sops.sem_op = -1;
	sops.sem_flg = 0;
	CHECK(semop(semId, &sops, 1) != -1);
}

void unlockSem(int sem_num){
	struct sembuf sops;
	sops.sem_num = (unsigned short)sem_num;
	sops.sem_op = +1;
	sops.sem_flg = 0;
	CHECK(semop(semId, &sops, 1) != -1);
}

void lockMutex() { lockSem(0);}
void unlockMutex() { unlockSem(0);}
void lockBarier1() { lockSem(1);}
void unlockBarier1() { unlockSem(1);}
void lockBarier2() { lockSem(2);}
void unlockBarier2() { unlockSem(2);}

int count(){
	int number;
	read_(&number);
	return number;
}

void increment() {
	int number;
	read_(&number);
	number++;
	write_(&number, 1);
}

void decrement() {
	int number;
	read_(&number);
	number--;
	write_(&number, 1);
}

void Barier1(){
	lockMutex();
	increment();
	if(count() == num_processes){
		int i;
		for(i = 0; i < num_processes; ++i)
			unlockBarier1();
	} 
	unlockMutex();
	lockBarier1();
}

void Barier2(){
	lockMutex();
	decrement();
	if(count() == 0){
		int i;
		for(i = 0; i < num_processes; ++i)
			unlockBarier2();
	}
	unlockMutex();
	lockBarier2();
}

void init(int numprocesses){
	sem_num = 3;
	num_processes = numprocesses;
	createShared();
	createSem(sem_num);
	CHECK((buf = (int*)shmat(shmId, NULL, 0)) != (int*) -1);
	unlockMutex();// aby bol inicializovany na 1 --- prvy beh
}