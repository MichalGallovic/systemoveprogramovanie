all: utility.o homework.o 

include ${CURDIR}/makefile.common

utility.o: utility.c utility.h

homework.o: homework.c homework.h utility.h

clean:
	rm -f *.o


