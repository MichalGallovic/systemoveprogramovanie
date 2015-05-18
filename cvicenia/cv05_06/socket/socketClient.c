#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include "commonSocket.h"

int main() {
	STUDENT student;

	//vytvorenie soketu cez ktory bude klient komunikovat
	//socket

	//vytvorenie spojenia zo serveroom
	//connect

	//nastavenie udajov, ktore budu poslane serveru
	SetStudent(&student, "MenoStudenta", 1);

	//poslanie udajov
	//write alebo send

	//prijatie udajov
	//read alebo recv

	//vypis prijatych udajov zo servera
	PrintStudent(&student);

	//zatvorenie soketu
	//close

	return EXIT_SUCCESS;
}
