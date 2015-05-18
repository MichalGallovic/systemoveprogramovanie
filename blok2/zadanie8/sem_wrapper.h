#ifndef _SEM_WRAPPER_H_
#define _SEM_WRAPPER_H_

#include "common.h"

union semun {
		int val;    		 /* Value for SETVAL */
    	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    	unsigned short  *array;  /* Array for GETALL, SETALL */
    	struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

typedef struct {
	int id;			// identifikator
	int count;		// pocet semaforov
	int * sequence;	// poradie semaforov 
} sem_wrapper;

sem_wrapper semObj;	// "objekt" spravujuci mnozinu semaforov

// inicializuje wrapper dany parametrom
void initSemWrapper(sem_wrapper * semObj, int semNum) {
	//  
	//semObj->sequence = (int *) malloc(semObj->count * sizeof(int));
	semObj->count = semNum;
	
	// vytvori mnozinu semaforov
	CHECK( (semObj->id = semget(IPC_PRIVATE, semObj->count, IPC_CREAT | S_IRUSR | S_IWUSR)) != -1 );
	
	// pole hodnot semaforov - ich pociatocny stav
	unsigned short * semValues = (unsigned short *) malloc(semObj->count * sizeof(unsigned short));
	
	// inicializuje semafory na nulovu hodnotu && nastavy default poradie semaforov - vzostupne 0,1...
	int semId;
	for(semId = 0; semId < semObj->count; semId++) {
		semValues[semId] = 0;
		//semObj->sequence[semId] = semId;
	}
	union semun semUnion;
	semUnion.array = semValues;
	CHECK( semctl(semObj->id, semObj->count, SETALL, semUnion) != -1);

	free(semValues);
	
	/*
	// nahodne premiesa poradie semaforov
	int firstPos, secondPos, tempValue;
	srand(time(NULL));
	for (semId = 0; semId < semObj->count; semId++) {
		firstPos = rand() % semObj->count;
		secondPos = rand() % semObj->count;
		
		tempValue = semObj->sequence[firstPos];
		semObj->sequence[firstPos] = semObj->sequence[secondPos];
		semObj->sequence[secondPos] = tempValue;
	}
	*/
}

// inicializuje globalny semObj
void initSem(int semNum) {
	initSemWrapper(&semObj, semNum);
}

void deinitSemWrapper(sem_wrapper * semObj) {
	CHECK( semctl(semObj->id, IPC_RMID, 0) != -1 );
	//free(semObj->sequence);
}

// destruktor - uvolni dynamicky alokovanu pamat
void deinitSem() {
	deinitSemWrapper(&semObj);	
}

// zablokuje alebo uvolni semafor semafor
void processSemWrapperOperation(sem_wrapper * semObj, int semId, int semOperation) {
	struct sembuf sops[1];	// pomocna premenna pre vykonavanie operacii nad semaformi
	sops[0].sem_num = semId;
	sops[0].sem_op = semOperation;
	sops[0].sem_flg = 0;	// nepouzijeme SEM_UNDO
	CHECK( semop(semObj->id, sops, 1) != -1 );
}

// zablokuje semafor
void waitSem(int semId) {
	processSemWrapperOperation(&semObj, semId, -1);
}

// uvolni semafor
void enableSem(int semId) {
	processSemWrapperOperation(&semObj, semId, 1);
}

// vstup - index do pola semaforov, vrati poradie daneho semafora
int getSemWrapperId(sem_wrapper * semObj, int semArrId) {
	if (semArrId >= semObj->count) semArrId = 0;
	return semObj->sequence[semArrId];
	//return semArrId;
}

// vstup - index do pola semaforov, vrati poradie daneho semafora
int getSemId(int semArrId) {
	return getSemWrapperId(&semObj, semArrId);
}

#endif
