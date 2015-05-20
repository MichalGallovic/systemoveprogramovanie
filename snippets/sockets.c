#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_ADDRESS "/tmp/soket_v_suborovom_systeme"
#define DATA_SIZE 10
#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }


void client() {
    char data[DATA_SIZE] = "ANAKONDA";

    int sock;
    struct sockaddr_un sockAddr;
    ssize_t receivedDataLength;
    int sockAddrLen;
    //vytvorenie soketu cez ktory bude klient komunikovat
    //socket
    CHECK( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) != -1 );
    //vytvorenie spojenia zo serveroom
    //connect
    sockAddr.sun_family = AF_UNIX;
    strcpy(sockAddr.sun_path, SOCKET_ADDRESS);
    sockAddrLen = SUN_LEN(&sockAddr);
    CHECK( connect(sock,(struct sockaddr *)&sockAddr, sockAddrLen) != -1);
    //nastavenie udajov, ktore budu poslane serveru


    //poslanie udajov
    //write alebo send
    CHECK( send(sock,data, sizeof(char)*DATA_SIZE, 0)  != -1);
    //prijatie udajov
    //read alebo recv
    CHECK( (receivedDataLength = read(sock, data, sizeof(char) * DATA_SIZE)) != -1);
    //vypis prijatych udajov zo servera
    if(receivedDataLength != sizeof(char)*DATA_SIZE) {
        fprintf(stderr, "CHYBA: neboli prijate kompletne udaje\n");
        exit(EXIT_FAILURE);
    }

    printf("[client] %s\n", data);

    //zatvorenie soketu
    //close
    CHECK( close(sock) != -1);

    exit(EXIT_SUCCESS);
}

void server() {
    char data[DATA_SIZE];

    int serverSocket;
    int clientSocket;
    ssize_t receivedLength;
    struct sockaddr_un serverAddr;
    //vytvorenie soketu
    CHECK( (serverSocket = socket(AF_UNIX, SOCK_STREAM, 0)) != -1 );
    //socket
    unlink(SOCKET_ADDRESS);
    //previazanie adresy soketu (mena suboru v suborovom systeme) na soket
    //bind
    strcpy(serverAddr.sun_path, SOCKET_ADDRESS);
    serverAddr.sun_family = AF_UNIX;

    CHECK( bind(serverSocket,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr_un)) != -1);
    //oznacenie soketu ako soketu, ktory bude pouzity na akceptovanie prihadzajucich ziadosti o pripojenie (funkciou accept)
    //a nastavenie maximalnej velkosti fronty pre prichadzajuce ziadosti o pripojenie
    //listen
    CHECK(listen(serverSocket,5) != -1 );

    //akceptacia ziadosti o pripojenie
    //accept
    CHECK( (clientSocket = accept(serverSocket, NULL, NULL))  != -1 );

    printf("[server]: Spojenie s klientom vytvorene\n");
    //citanie udajov od klienta:
    //read alebo recv

    CHECK( (receivedLength = recv(clientSocket, data, sizeof(char)*DATA_SIZE, MSG_WAITALL) ) != -1 );
    if(receivedLength != sizeof(char)*DATA_SIZE ) {
        fprintf(stderr, "CHYBA: nebol prijate kompletne udaje\n");
        exit(EXIT_FAILURE);
    }

    printf("[server] %s\n", data);

    //zmena nacitanych udajov
    strncpy(data,"JA NEVIEM",9);

    //poslanie zmenenych udajov
    //write alebo send
    CHECK( write(clientSocket, data, sizeof(char)*DATA_SIZE) != -1);
    //zatvorenie soketu
    //close
    CHECK(close(clientSocket) != -1 );

    //odstranenie soketu zo suboroveho systemu
    //unlink
}

int main() {
    pid_t pid;

    pid = fork();
    switch(pid) {
        case -1:
            perror("fork");
            break;
        case 0:
            sleep(2);
            client(); //detsky process
            break;
        default:
            server(); //rodicovsky process
            wait(NULL);
    }

    return EXIT_SUCCESS;
}