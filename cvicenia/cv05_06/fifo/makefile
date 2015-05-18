all: common fifoSender fifoReceiver 

include ${CURDIR}/../common/makefile.common


commonFifo.h: ../common/student.h ../common/utility.h


fifoSender.o: fifoSender.c commonFifo.h

fifoReceiver.o: fifoReceiver.c commonFifo.h


fifoSender: fifoSender.o ../common/student.o

fifoReceiver: fifoReceiver.o ../common/student.o


clean:
	rm -f *.o
	rm -f fifoSender
	rm -f fifoReceiver
	cd ../common && make clean

