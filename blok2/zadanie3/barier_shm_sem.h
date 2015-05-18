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
static int shmId;
static int semId;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

void deleteShared() { CHECK(shmctl(shmId,IPC_RMID,0) != -1); }
void deleteSem() { CHECK(semctl(semId, 0, IPC_RMID) != -1); }

void createShared(size_t size) {
    CHECK((shmId = shmget(IPC_PRIVATE, size, 0660) ) != -1);
}

void createSem(int semnumber) {
    int i = 0;
    union semun sem_un;
    unsigned short  *sem_val;
    sem_val = (unsigned short *)malloc(sizeof(unsigned short) * semnumber);
    CHECK( (semId = semget(IPC_PRIVATE, semnumber, IPC_CREAT | 0600)) != -1);
    for(i = 0; i < semnumber;i++) {
        sem_val[i] = 0;
    }
    sem_un.array = sem_val;
    CHECK(semctl(semId, 0,SETALL, sem_un) != -1);
    free(sem_val);  
}



void init(int semnumber) {
    semnumber++;
    createShared(MAX_SIZE);
    createSem(semnumber);
    CHECK((buf = (char*)shmat(shmId,NULL,0)) != (char*) -1);
    int buffer = '\0';
    memset(buf, &buffer,1);

}

void teardown() {
    CHECK(shmdt(buf) != -1);
    deleteShared();
    deleteSem();
}

void lockSem(int semnumber) {
    struct sembuf op;
    op.sem_num = (unsigned short)semnumber;
    op.sem_op = -1;
    op.sem_flg = 0;
    CHECK(semop(semId,&op, 1) != -1);
}

void unlockSem(int semnumber) {
    struct sembuf op;
    op.sem_num = semnumber;
    op.sem_op = +1;
    op.sem_flg = 0;
    CHECK(semop(semId, &op, 1) != -1);
}

void lockMutex() { lockSem(0); }
void unlockMutex() { unlockSem(0); }

void write_(int *buffer, size_t size) {
    memcpy(buf, buffer, size);
}
void read_(int* buffer) {
    memcpy(buffer, buf, sizeof(int));
}
