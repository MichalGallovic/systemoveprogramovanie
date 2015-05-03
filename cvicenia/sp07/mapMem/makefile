all: common mapMem

include ${CURDIR}/../common/makefile.common

mapMem.o: mapMem.c ../common/homework.h ../common/utility.h

mapMem: mapMem.o ../common/homework.o ../common/utility.o

clean:
	rm -f *.o
	rm -f mapMem
	cd ../common && make clean