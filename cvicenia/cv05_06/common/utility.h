#ifndef UTILITY_H
#define	UTILITY_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

//makro pre jednoduchsiu kontrolu navratovej hodnoty volania funkcie a vypis popisu chyby
#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

#endif	//UTILITY_H

