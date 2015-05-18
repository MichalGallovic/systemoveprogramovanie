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

	//vytvorenie fifo

	//otvorenie fifo na zapis

	//odstranenie fifo zo suboroveho systemu

	//posielanie udajov
	SetStudent(&student, "Meno1", 1); //nastavenie udajov, ktore sa budu posielat
	//poslanie udajov o studentovi
	SetStudent(&student, "Meno2", 2); //nastavenie udajov, ktore sa budu posielat
	//poslanie udajov o studentovi

	//zatvorenie fifo
		
	return EXIT_SUCCESS;
}

