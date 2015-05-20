#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

#define FIFO_NAME "/tmp/fifofifo"
#define DATA_SIZE 20

void writeDATA() {
    char data[DATA_SIZE] = "12345";

    int fd;
    umask(0);
    //vytvorenie fifo
    CHECK(mkfifo(FIFO_NAME, 0660) != -1 || errno == EEXIST);
    //otvorenie fifo na zapis
    if((fd = open(FIFO_NAME,O_WRONLY)) == -1) {
        perror("FIFO sender open");
        exit(EXIT_FAILURE);
    }


    CHECK(write(fd,data,DATA_SIZE*sizeof(char)) != -1);
    CHECK(write(fd,data,DATA_SIZE) != -1);
    CHECK(write(fd,data,DATA_SIZE) != -1);
    sleep(1);
    //zatvorenie fifo
    CHECK(close(fd) != -1);
    printf("WRITE FIFO ZATVORENE\n");
}

void readDATA() {
    int fd, readResult;
    char data[DATA_SIZE];
    umask(0);
    CHECK(mkfifo(FIFO_NAME, 0660) != -1 || errno == EEXIST);
    // otvorenie fifo na zapis
    CHECK( (fd = open(FIFO_NAME,O_RDONLY) ) != -1);
    CHECK(unlink(FIFO_NAME) != -1);
    //citanie udajov a vypis udajov
    while( (readResult = read(fd, data, DATA_SIZE*sizeof(char) )) > 0) {
        fprintf(stderr, "%s\n",data );
    }
    if(readResult == -1 ){
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("vsetky udaje prijate\n");

    //zatvorenie fifo
    CHECK(close(fd) != -1);
    printf("READ FIFO ZATVORENE\n");
    exit(EXIT_SUCCESS);
}

int main()
{
    pid_t pid = fork();
    switch(pid) {
        case -1:
            perror("Fork");
            exit(EXIT_FAILURE);
        case 0:
            readDATA();
            break;
        default:
            writeDATA();
            wait(NULL);
            break;
    }

    return EXIT_SUCCESS;
}