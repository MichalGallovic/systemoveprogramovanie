all: common pipe 

include ${CURDIR}/../common/makefile.common

pipe.o: pipe.c ../common/student.h

pipe: pipe.o ../common/student.o

clean:
	rm -f *.o
	rm -f pipe
	cd ../common && make clean

