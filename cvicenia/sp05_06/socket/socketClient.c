#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include "commonSocket.h"

int main() {
	STUDENT student;
	int sock;
	struct sockaddr_un socket_addr;
	ssize_t received;

	CHECK((sock = socket(AF_UNIX, SOCK_STREAM, 0)));

	socket_addr.sun_family = AF_UNIX;
	strcpy(socket_addr.sun_path, SOCKET_ADDRESS);
	CHECK(connect(sock, (struct sockaddr*) &socket_addr, sizeof(struct sockaddr_un)));
	
	SetStudent(&student, "MenoStudenta", 1);

	CHECK(send(sock, &student, sizeof(STUDENT), 0));

	CHECK((received = recv(sock, &student, sizeof(STUDENT), MSG_WAITALL)));
	if(received != sizeof(STUDENT)){
		fprintf(stderr, "ERROR: request is not satisfied");
		return EXIT_FAILURE;
	}
	
	PrintStudent(&student);

	CHECK(close(sock));

	return EXIT_SUCCESS;
}
