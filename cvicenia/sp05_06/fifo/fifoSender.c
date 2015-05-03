#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "commonFifo.h"

int main(int argc, char * argv[]) 
{
	STUDENT student;
	int fifo;
	
	umask(0);
	if(	mkfifo(FIFO_NAME, 0660) == -1 && errno != EEXIST){
		perror("mkfifo");
		return EXIT_FAILURE;
	}

	CHECK((fifo = open(FIFO_NAME, O_WRONLY)));
	CHECK(unlink(FIFO_NAME));

	SetStudent(&student, "Meno1", 1); 
	write(fifo, &student, sizeof(STUDENT));
	SetStudent(&student, "Meno2", 2); 
	write(fifo, &student, sizeof(STUDENT));

	CHECK(close(fifo));
	
	return EXIT_SUCCESS;
}

