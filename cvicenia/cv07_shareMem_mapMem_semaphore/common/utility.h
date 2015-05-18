#ifndef UTILITY_H
#define	UTILITY_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

//makro pre jednoduchsiu kontrolu navratovej hodnoty volania funkcie a vypis popisu chyby
#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

//vytlaci retazec podobne ako printf, ale na zaciatok vytlaci identifikaciu procesu (ID procesu a priradene pismeno) a dvojbodku 
void print(char * formatString, ...);

#endif	//UTILITY_H
