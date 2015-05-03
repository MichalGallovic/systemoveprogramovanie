#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include "commonSocket.h"


int main() {
	STUDENT student;
	int server_socket;
	int client_socket;
	struct sockaddr_un server_addr;
	ssize_t received;

	CHECK((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)));
		 
	memset(&server_addr, 0, sizeof(struct sockaddr_un));	
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCKET_ADDRESS);
	CHECK(bind(server_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_un)));

	CHECK(listen(server_socket, 10));

	CHECK((client_socket = accept(server_socket, NULL, NULL))); 
	
	CHECK((received = recv(client_socket, &student, sizeof(STUDENT), MSG_WAITALL)));
	if(received != sizeof(STUDENT)){
		fprintf(stderr, "ERROR: request is not satisfied");
		return EXIT_FAILURE;
	}

	PrintStudent(&student);
	
	student.id++;
	
	CHECK(send(client_socket, &student, sizeof(STUDENT), 0));

	CHECK(close(server_socket));

	CHECK(unlink(SOCKET_ADDRESS));

	return EXIT_SUCCESS;
}
