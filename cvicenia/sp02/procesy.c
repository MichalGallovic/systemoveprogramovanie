#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

char* get_timestamp ()
{
  time_t now = time (NULL);
  return asctime (localtime (&now));
}

void child() {
	int file;
	char buf[11];
	buf[10]= '\0';

	printf("child -  pid: %i, parent pid: %i\n", getpid(), getppid());

	if((file = open("udaje.txt", O_WRONLY | O_CREAT | O_APPEND, 0666 )) == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	char* timestamp = get_timestamp ();
	size_t length = strlen (timestamp);

	if(write (file, timestamp, length) > 0) {
		perror("write");
	}	

//	while( 1 ){
//		switch( read(file, buf, 10) ){
//			case 0:
//				goto end;
//				break;
//			case -1:
//				perror("read");
//				exit(EXIT_FAILURE);
//			default: 
//				printf("%s", buf);
//				sleep(1);
//				break;
//		}
//	}
//end:
	if(close(file) == -1){
		perror("close");
		exit(EXIT_FAILURE);
	}
}

void parent(pid_t childPid) {
	int file;
	char buf[11];
	buf[10]= '\0';

	printf("parent - pid: %i, child pid: %i\n", getpid(), childPid); 

	if((file = open("udaje.txt", O_RDONLY)) == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	while( 1 ){
		switch( read(file, buf, 10) ){
			case 0:
				goto end;
				break;
			case -1:
				perror("read");
				exit(EXIT_FAILURE);
			default: 
				printf("%s", buf);
				//sleep(1);
				break;
		}
	}
end:
	if(close(file) == -1){
		perror("close");
		exit(EXIT_FAILURE);
	}
}


int main()
{
	pid_t pid;
	
	pid = fork();
	switch(pid){
		case -1:
			exit(EXIT_FAILURE);
		case 0:
			child();
			exit(EXIT_SUCCESS);
		default:
			wait(NULL);
			parent(pid);
			exit(EXIT_SUCCESS);
	}

}
