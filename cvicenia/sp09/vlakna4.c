#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

//kluc pre pristup ku specifickym premennym vlakien
pthread_key_t dataKey;

//pracovna struktura pre vlakna
typedef struct {
	char label;  //oznacenie vlakna
	int counter; //pocet opakovani cyklu vo vlakne
} Data;

//vytvorenie a inicializacia pracovnej struktury pre pracovne vlakna
Data * DataCreate(int threadOrder) {
	Data * threadData = malloc(sizeof(Data));
	assert( threadData != NULL );
	threadData->label = 'A' + threadOrder;
	threadData->counter = 0;
	return threadData;
}

//vypis obsahu pracovnej struktury
void DataPrint(Data * data) {
	printf("label: %c, counter: %d\n", data->label, data->counter);
}

//vypis podobne ako funkciou printf, ale na zaciatku prida oznacenie pracovneho vlakna
//oznacenie pracovneho vlakna ziska pomocou kluca ku specifickej premennej vlakna
void Print(char* formatString, ...) {
	Data * data = (Data*) pthread_getspecific(dataKey);
	assert( data != NULL);

	va_list args;
	va_start(args, formatString);
	printf("%c: ", data->label);
	vprintf(formatString, args);
	va_end(args);
}

//vypis obsahu struktury a dealokacia struktury
void DataPrintAndDestroy(Data * data) {
	printf("%c: vlakno ukoncene, counter: %d\n", data->label, data->counter);
	free(data);
}

//Funkcia, ktoru vykonavaju pracovne vlakna
void * Worker(void *inputData) {
	Data *data = (Data*)inputData;
	data->counter = 0;

	assert(pthread_setspecific(dataKey, inputData) == 0);

	for(;;)	{
		data->counter++;
		Print("counter: %i\n", data->counter);
		sleep(rand()%5);
	}
	return NULL;
}

//hlavne vlakno
int main(int argc, char *argv[]) {
	int const THREAD_COUNT = 5;	//pocet vlakien
	pthread_t id[THREAD_COUNT]; //ID vlakien
	int i;

	//inicializacia generatora nahodnych cisiel
	srand(time(NULL));

	//vytvorenie kluca pre spedifike premenne
	assert(pthread_key_create(&dataKey, (void (*)(void*))DataPrintAndDestroy) == 0);	

	for(i = 0; i < THREAD_COUNT; i++ ) {
		Data * data = DataCreate(i); 
		assert(pthread_create(&id[i], NULL, Worker,(void*)data) == 0);
	}

	//cakanie hlavneho vlakna
	sleep(10);

	//synchronne zrusenie vlakien
	for(i = 0; i < THREAD_COUNT; i++ ) 
		assert(pthread_cancel(id[i]) == 0);

	//cakanie na ukoncenie vlakien
	for(i = 0; i < THREAD_COUNT; i++ ) 
		assert(pthread_join(id[i], NULL) == 0);

	printf("vsetky pracovne vlakna ukoncene\n");

	return EXIT_SUCCESS;
}



