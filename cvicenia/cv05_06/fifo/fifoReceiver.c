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
	STUDENT student; //struktura pre docasne ulozenie precitanych udajov 

	//vytvorenie fifo

	//otvorenie fifo pre citanie

	//citanie udajov a vypis udajov
	//while( DOPLNIT ) {
	//	PrintStudent(&student);
	//}
	printf("vsetky udaje prijate\n");

	//zatvorenie fifo

	return EXIT_SUCCESS;
}

