#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include "commonSocket.h"


int main() {
	STUDENT student;

	//vytvorenie soketu
	//socket
	
	//previazanie adresy soketu (mena suboru v suborovom systeme) na soket
	//bind

	//oznacenie soketu ako soketu, ktory bude pouzity na akceptovanie prihadzajucich ziadosti o pripojenie (funkciou accept) 
	//a nastavenie maximalnej velkosti fronty pre prichadzajuce ziadosti o pripojenie
	//listen

	//akceptacia ziadosti o pripojenie
	//accept

	//citanie udajov od klienta
	//read alebo recv

	//vypis prijatych udajov
	PrintStudent(&student);
	
	//zmena nacitanych udajov
	student.id++;

	//poslanie zmenenych udajov
	//write alebo send

	//zatvorenie soketu
	//close

	//odstranenie soketu zo suboroveho systemu
	//unlink

	return EXIT_SUCCESS;
}
