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

#define MAX_SIZE 4096

static char* buf;

static int shmId; //id zdielanej pamate
static int semId; //id mnoziny semaforov
static int sem_num;

union semun {
		int val;    		 /* Value for SETVAL */
    	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    	unsigned short  *array;  /* Array for GETALL, SETALL */
    	struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};


void deleteShared(){ CHECK(shmctl(shmId, IPC_RMID, 0) != -1);} 

void deleteSem(){ CHECK(semctl(semId, 0, IPC_RMID) != -1);}

void createShared(size_t size){
	CHECK((shmId = shmget(IPC_PRIVATE, size, 0660)) != -1);
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

void write_(char* buffer, size_t size){
	memcpy(buf, buffer, size);
}

void read_(char* buffer){
	memcpy(buffer, buf, strlen(buf));
}

void init(int semnum){
	sem_num = semnum;
	createShared(MAX_SIZE);
	createSem(sem_num);
	CHECK((buf = (char*)shmat(shmId, NULL, 0)) != (char*) -1);
	char buffer = '\0'; /// aby bolo iste ze je prazdny
	memcpy(buf, &buffer, 1);
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
	switch(semop(semId, &sops, 1)){
		case 0:
			break;
		case -1:
			if(errno == EINTR) {
				CHECK(semop(semId, &sops, 1) != -1); //handluje prichod signalu
				return;
			}
		default:
			fprintf(stderr, "Error: semop: %s\n", strerror(errno)); 
			exit(EXIT_FAILURE);
	}
}

void unlockSem(int sem_num){
	struct sembuf sops;
	sops.sem_num = (unsigned short)sem_num;
	sops.sem_op = +1;
	sops.sem_flg = 0;
	switch(semop(semId, &sops, 1)){
		case 0:
			break;
		case -1:
			if(errno == EINTR) {
				CHECK(semop(semId, &sops, 1) != -1);  //handluje prichod signalu
				return;
			}
		default:
			fprintf(stderr, "Error: semop: %s\n", strerror(errno)); 
			exit(EXIT_FAILURE);
	}
}
