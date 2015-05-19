#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef enum {
	UNSET = 0,
	SET = 1
} OPTION;

typedef struct {
	OPTION h;
	OPTION i;
	char* input_file;
	OPTION o;
	char* output_file;
	OPTION s;
	char* ssize;
	int size;
} ARGS;

typedef enum {
	FALSE = 0,
	TRUE = 1
} Bool;

typedef struct {
	Bool signalReceived;   //priznak ci bol signal doruceny
	pthread_mutex_t mutex; //zamok pre synchronizaciu
	pthread_cond_t cond;   //podmienkova premenna pre synchronizaciu
} SignalControl;

typedef struct {
	char* data; //udaje v buffer-y
	char* input_file;
	char* output_file;    
	int capacity; //maximalny pocet udajov v buffer-y
	size_t size; //size of read data
	pthread_mutex_t mutex; //synchronizacia pristupu ku kritickej oblasti
	sem_t  waitedPaste; //cakajuci producenti (ked je plny buffer)
	sem_t  waitedCopy; //cakajuci konzumenti (ked je prazdny buffer)
} Buffer;

typedef struct {
	char* data; 
	int fd; 
} ThreadData;
 
void printHelpAndExit(FILE* stream, int exitCode){
	fprintf(stream, "Usage: parametre [-h] -i <subor1> -o <subor2> -s <velkost_buffra> \n");
	fprintf(stream, "Zadanie 3-02\n");
	fprintf(stream, "Prepinace:\n");
	fprintf(stream, " -h, --help   vypise help\n");
	fprintf(stream, " -i, --input   vstupny subor\n");
	fprintf(stream, " -o, --output   vystupny subor\n");
	fprintf(stream, " -s, --size   velkost buffera\n");
	exit(exitCode);
}

void parseArgs(int argc, char * argv[], ARGS * args) {
	int opt;

	args->h = UNSET;
	args->i = UNSET;
	args->s = UNSET;
	args->o = UNSET;
	args->input_file = NULL;
	args->output_file = NULL;
	args->ssize = NULL;
	args->size = 0;
	
	static struct option long_options[] = {
                   						  {"help", 0, NULL, 'h'},
                   						  {"input", 1, NULL, 'i'},
                   						  {"output", 1, NULL, 'o'},
                   						  {"size", 1, NULL, 's'},
                   						  {0, 0, 0, 0}
               							  };
	int option_index = 0;

	do {
		opt = getopt_long(argc, argv, ":hi:o:s:", long_options, &option_index);
		switch (opt) {
		case 'h':
			args->h = SET;
			printHelpAndExit(stderr, EXIT_SUCCESS);
			break;
		case 'i':
			args->i = SET;
			args->input_file = optarg;
			break;
		case 'o':
			args->o = SET;
			args->output_file = optarg;
			break;
		case 's':
			args->s = SET;
			args->ssize = optarg;
			break;
		case '?': 	
			fprintf(stderr,"Neznama volba -%c\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		case ':': 	
			fprintf(stderr, "Nebol zadany argument prepinaca '-%c'\n", optopt);
			printHelpAndExit(stderr, EXIT_FAILURE);
		default:
			break;
		}
		
	} while(opt != -1);

	while(optind < argc ) {
		printf("Debug:    non-option ARGV-element: %s\n", argv[optind++]);
	}
}

void validateArgs(ARGS * args) {
	if(args->s == UNSET){
			fprintf(stderr, "Prepinac -s je povinny!\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
	}
	if(args->i == UNSET){
			fprintf(stderr, "Prepinac -i je povinny!\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
	}
	if(args->o == UNSET){
			fprintf(stderr, "Prepinac -o je povinny!\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
	}
	if(args->s == SET){
		if(sscanf(args->ssize, "%d", &args->size) <= 0 ) {
			fprintf(stderr, "Argument prepinaca -s nie je cislo!\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
		}
	}
	if(args->i == SET && args->input_file == NULL){
			fprintf(stderr, "Argument prepinaca -i je prazdny!\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
	}
	if(args->o == SET && args->output_file == NULL){
			fprintf(stderr, "Argument prepinaca -o je prazdny!\n");
			printHelpAndExit(stderr, EXIT_FAILURE);
	}
}



Buffer* bufferCreate(int capacity, char* in, char* out)
{
	Buffer * buffer;
	
	buffer = (Buffer*) malloc(sizeof(Buffer));
	assert(buffer != NULL);
	buffer->data = (char*) malloc(sizeof(char)*capacity);
	assert(buffer->data != NULL);
	buffer->capacity = capacity;
	buffer->input_file = in;
	assert(buffer->input_file != NULL);
	buffer->output_file = out;
	assert(buffer->output_file != NULL);

	//inicializacia zamku podmienkovych premennych	
	assert(pthread_mutex_init( & buffer->mutex, 0) == 0 );
	assert(sem_init( & buffer->waitedPaste, 0, 1) == 0 );//kopirovat uz moze
	assert(sem_init( & buffer->waitedCopy, 0, 0) == 0 ); 
	return buffer;
}

void bufferDestroy(void* data)
{
	Buffer *buffer = (Buffer*)data;
	//zrusenie zamku a semaforov
	assert(sem_destroy(&buffer->waitedPaste) == 0 );
	assert(sem_destroy(&buffer->waitedCopy) == 0 );
	assert(pthread_mutex_destroy(&buffer->mutex) == 0 );

	free(buffer->data); 
	free(buffer);
}

void threadDestroy(void* data)
{
	ThreadData *buffer = (ThreadData*)data;
	assert(close(buffer->fd) != -1);
	free(buffer->data);
	free(data);
}

void bufferWrite(Buffer* buffer, char* data, size_t size)
{
	assert( sem_wait( & buffer->waitedPaste) == 0); 
	assert( pthread_mutex_lock( & buffer->mutex) == 0 );
	memcpy(buffer->data, data, size);
	buffer->size = size;
	assert( pthread_mutex_unlock( & buffer->mutex) == 0 );
	assert( sem_post( & buffer->waitedCopy) == 0);
}

void bufferRead(Buffer* buffer, char* data, size_t *size)
{
	assert( sem_wait( &buffer->waitedCopy) == 0);
	assert( pthread_mutex_lock( & buffer->mutex) == 0 );
	memcpy(data, buffer->data, buffer->size);
	*size = buffer->size;
	assert(pthread_mutex_unlock(&buffer->mutex) == 0);
	assert(sem_post(&buffer->waitedPaste) == 0);
}

void* killer(void *data){
	SignalControl * signalControl = (SignalControl*) data;
	sigset_t processedSignals; 
	sigset_t allSignals;       
	int receivedSignal;        
	
	assert(sigfillset(&allSignals) != -1);
	assert(pthread_sigmask(SIG_BLOCK, &allSignals, NULL) == 0 );
	
	sigemptyset( &processedSignals);
	sigaddset( &processedSignals, SIGINT);
	
	assert( sigwait( &processedSignals, &receivedSignal) == 0);
	printf("signal waiter: SIGINT\n");
	assert( pthread_mutex_lock(&signalControl->mutex) == 0);
	signalControl->signalReceived = TRUE;
	assert( pthread_cond_signal(&signalControl->cond) == 0); //poslanie signalu vsetky threadom
	assert( pthread_mutex_unlock(&signalControl->mutex) == 0);
	return NULL;
}

void* paste(void *data){
	Buffer *buffer = (Buffer*)data;
	ThreadData *thread_data = (ThreadData*)malloc(sizeof(ThreadData));
	thread_data->data = (char*)malloc(sizeof(char)*buffer->capacity);
	size_t size;

	pthread_cleanup_push((void (*)(void*)) threadDestroy, thread_data); //cleanup pre vlakno

	assert((thread_data->fd = open(buffer->output_file, O_WRONLY | O_CREAT | O_CREAT, 0666)) != -1);
	while(1){
		bufferRead(buffer, thread_data->data, &size);
		// fprintf(stderr, "Writed: %s %d\n", thread_data->data, (int)size);
		assert(write(thread_data->fd, thread_data->data, size) == (ssize_t)size);
		if(size < (size_t)buffer->capacity) break; //ked je precitany subor
	}
	pthread_cleanup_pop(1);
	fprintf(stderr, "Killing paste thread\n");
	assert(kill(getpid(), SIGINT) == 0); //zrusim vsetky thready
	return NULL;
}

void* copy(void *data){
	Buffer *buffer = (Buffer*)data;
	ThreadData *thread_data = (ThreadData*)malloc(sizeof(ThreadData));
	thread_data->data = (char*)malloc(sizeof(char)*buffer->capacity);
	size_t size;

	pthread_cleanup_push((void (*)(void*)) threadDestroy, thread_data); //cleanup pre vlakno

	assert((thread_data->fd = open(buffer->input_file, O_RDONLY)) != -1);
	while((size = read(thread_data->fd, thread_data->data, buffer->capacity))){
		// fprintf(stderr, "Readed: %s %d\n",thread_data->data, (int)size);
		bufferWrite(buffer, thread_data->data, size);
	}
	pthread_cleanup_pop(1);

	fprintf(stderr, "Killing copy thread\n");
	return NULL;
}

int main(int argc, char * argv[]){
 	ARGS args;
 	Buffer * buffer;
 	pthread_t id[3];
 	int i;
 	sigset_t blockedSignals;
 	SignalControl signalControl = { 
 									FALSE, 
 									PTHREAD_MUTEX_INITIALIZER, 
 									PTHREAD_COND_INITIALIZER
 								  };

	parseArgs(argc, argv, &args);
	validateArgs(&args);

	// Zablokovanie signalu, potom ho odblokujem v druhu threade
	sigemptyset( &blockedSignals);
	sigaddset( &blockedSignals, SIGINT);
	assert( pthread_sigmask(SIG_BLOCK, &blockedSignals, NULL) == 0 );

	buffer = bufferCreate(args.size, args.input_file, args.output_file);

	pthread_cleanup_push((void (*)(void*)) bufferDestroy, buffer); //cleanup pre hlavne vlakno

	assert( pthread_create( &id[0], NULL, &killer, &signalControl) == 0);
	assert( pthread_create( &id[1], NULL, &copy, buffer) == 0);
	assert( pthread_create( &id[2], NULL, &paste, buffer) == 0);

	assert( pthread_mutex_lock( &signalControl.mutex) == 0 );
	while( ! signalControl.signalReceived ) {
		assert( pthread_cond_wait( &signalControl.cond, &signalControl.mutex) == 0 );
	}
	assert( pthread_mutex_unlock( &signalControl.mutex) == 0 );

	int temp;
	//posielanie ukoncenie vlaknam
	for(i = 0; i < 3; i++ ) {
		temp = pthread_cancel(id[i]); //chujove hodnoty to vracia, pretoze bud je to nula alebo hodnota chyby
		if(temp != 0 && temp != ESRCH) //bud ho zrusi alebo uz neexistuje
			fprintf(stderr, "cancel error:%d", errno);
	}
	//cakanie na ukoncenie vlakien
	for(i = 0; i < 3; i++ ) {
		if((pthread_join(id[i], NULL) != 0) && errno != ESRCH) //bud nan caka alebo uz neexistuje
			fprintf(stderr, "join error:%d", errno);
	}

	pthread_cleanup_pop(1);
	return EXIT_SUCCESS;
}
