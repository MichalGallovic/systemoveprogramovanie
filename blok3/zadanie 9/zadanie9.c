#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_cond = PTHREAD_COND_INITIALIZER; // pre 2 inkrementujuce vlakna
pthread_cond_t null_pls = PTHREAD_COND_INITIALIZER;

void *functionCount1();
void *functionCount2();
void *function3();
int count = 0;        //pociatocny count
int COUNT_DONE  = 0;  //konecny count = ziskam ako argument a nastavim v parseArgs
char * s = ""; // pomocny string na preberanie argumentu

void parseArgs(int argc, char * argv[]){

int opt;

static struct option long_options[] = { {"help", 0, NULL, 'h'}

					};
int option_index = 0;

do {
	opt = getopt_long(argc, argv, "h", long_options, &option_index);


switch(opt){
		
	case 'h': 
	printf("help");
	break;
	
	default:
	break;

}

} while (opt != -1);

if (optind < argc ){

 s= argv[argc -1];
 COUNT_DONE = atoi(s);

 printf("\nNacital som a idem od 0 po  %d \n", COUNT_DONE);

}
 	return;
}


int main(int argc, char * argv[])
{

parseArgs(argc, argv);

pthread_t thread1, thread2, thread3;

pthread_create( &thread1, NULL, &functionCount1, NULL);
pthread_create( &thread2, NULL, &functionCount2, NULL);
pthread_create( &thread3, NULL, &function3, NULL);

pthread_join( thread1, NULL); 
pthread_join( thread2, NULL); 
pthread_join( thread3, NULL);

exit(0);
return EXIT_SUCCESS;
}
 
//tu spracuvam PARNE
// ak je neparne, cakam na 2.thread aby zmenil a poslal signal
void *functionCount1(){ 
while(1){

// Lock mutex and then wait for signal to relase mutex
pthread_mutex_lock( &condition_mutex );

//cakam na signal kym robi f2 
while(count % 2 == 1)
pthread_cond_wait( &condition_cond, &condition_mutex ); 

//mam signal, takze idem robit

count++;
if (count <= COUNT_DONE) printf("V1 -> %d\n",count);
pthread_mutex_unlock( &condition_mutex );

if(count >= COUNT_DONE) {
		//pthread_mutex_unlock( &condition_mutex);
	//	return (NULL); }
//	break;
			}
 	}

return NULL;
 }
// tu spracuvam NEPARNE
//ak pride PARNE poslem signal a NIC VIAC
void *functionCount2(){ 

while(1){
	pthread_mutex_lock( &condition_mutex );

//ak pride parne cislo, posli signal f1 nech pracuje
if ( count % 2 == 0 && count < COUNT_DONE )

	pthread_cond_signal( &condition_cond ); 

	//ak som dosiahol max posli signal f3 nech nuluje
	else if (count >= COUNT_DONE) {
	pthread_mutex_unlock( &condition_mutex);
	pthread_cond_signal( &null_pls);
	//return NULL;
	//break;
	}

	//else (mam pracovat ja) pracujem ...
	else {
	count++;
	printf	("V2 --> %d\n",count);
	}

pthread_mutex_unlock( &condition_mutex ); 

}
return NULL;
}

void *function3(){
	while(1){	
	pthread_mutex_lock( &condition_mutex);
	while(count < COUNT_DONE) pthread_cond_wait( &null_pls, &condition_mutex);
	if (count >= COUNT_DONE)
	{count = 0;
	printf("V3 nulujem count\n");
	}
	pthread_mutex_unlock( &condition_mutex);
	}
	return(NULL);
}
