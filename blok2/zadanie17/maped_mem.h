#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

#define MAX_SIZE 4096

static char* buf;

static int fileDescr; //
static char* file_name;
static int semId; //id mnoziny semaforov
static int sem_num;

union semun {
		int val;    		 /* Value for SETVAL */
    	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    	unsigned short  *array;  /* Array for GETALL, SETALL */
    	struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

void deleteMaped(){ 	
	CHECK(close(fileDescr) != -1);
	CHECK(unlink(file_name) != -1);
} 

void deleteSem(){ CHECK(semctl(semId, 0, IPC_RMID) != -1);}

void createMaped(size_t size){
	CHECK((fileDescr = mkstemp(file_name)) != -1);
	CHECK(lseek(fileDescr, size, SEEK_SET) != -1);
	CHECK(write(fileDescr, "", 1) != -1 );
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

void init(int semnum, char* filename){
	file_name = filename;
	sem_num = semnum;
	createMaped(MAX_SIZE);
	createSem(sem_num);
	CHECK((buf = (char *)mmap(NULL, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescr, 0)) != (char *) -1);
	// CHECK(close(fileDescr) != -1);
	char buffer = '\0'; /// aby bolo iste ze je prazdny
	memcpy(buf, &buffer, 1);
}

void teardown(){
	CHECK(munmap((void *) buf, MAX_SIZE) != -1);
	deleteMaped();
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