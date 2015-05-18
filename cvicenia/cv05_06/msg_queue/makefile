all: common msgQueueSender msgQueueReceiver

include ${CURDIR}/../common/makefile.common


commonMsgQueue.h: ../common/student.h ../common/utility.h


msgQueueSender.o: msgQueueSender.c commonMsgQueue.h

msgQueueReceiver.o: msgQueueReceiver.c commonMsgQueue.h


msgQueueSender: msgQueueSender.o ../common/student.o

msgQueueReceiver: msgQueueReceiver.o ../common/student.o


clean:
	rm -f *.o
	rm -f msgQueueSender
	rm -f msgQueueReceiver
	cd ../common && make clean

