all: common shareMem

include ${CURDIR}/../common/makefile.common

shareMem.o: shareMem.c ../common/homework.h ../common/utility.h 

shareMem: shareMem.o ../common/homework.o ../common/utility.o 

clean:
	rm -f *.o
	rm -f shareMem
	cd ../common && make clean
	 
