all: common socketServer socketClient

include ${CURDIR}/../common/makefile.common


commonSocket.h: ../common/student.h ../common/utility.h


socketServer.o: socketServer.c commonSocket.h

socketClient.o: socketClient.c commonSocket.h


socketServer: socketServer.o ../common/student.o

socketClient: socketClient.o ../common/student.o


clean:
	rm -f *.o
	rm -f socketServer
	rm -f socketClient
	cd ../common && make clean

