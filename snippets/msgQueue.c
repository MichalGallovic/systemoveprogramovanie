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


#define CHECK(command) if( ! (command) ) { fprintf(stderr, "Error: '%s' at %s line %d: %s\n", #command, __FILE__, __LINE__, strerror(errno)); exit(EXIT_FAILURE); }

#define MSG_QUEUE_KEY 424242     //kluc identifikujuci frontu sprav
#define MSG_TYPE_DATA 2      //typ spravy pre poslanie informacii o studentovi
#define MSG_TYPE_COUNT 1
//struktura pre spravu obsahujucu informacie o studentovi (definovat podla manualu)
typedef struct {
    long mtype;       /* message type, must be > 0 */
    char data[10];  /* message data */               //informacie o studentovi
} MSG_DATA;

typedef struct {
    long mtype;
    int count;
} MSG_DATA_COUNT;

void runReader() {
    int msgQueueId; //id fornty sprav

    MSG_DATA msg;
    MSG_DATA_COUNT msgCount;

    //podla kluca ziskame id fronty sprav
    //(analogia ku ziskaniu deskriptora suboru, identifikovaneho cestou v suborovm systeme)
    CHECK( (msgQueueId=msgget(MSG_QUEUE_KEY, S_IRUSR | S_IWUSR)) != -1);


    CHECK( msgrcv(msgQueueId, &msgCount, sizeof(int), MSG_TYPE_COUNT, 0) != -1);
    int i;
    for(i=0; i<msgCount.count; i++) {
        CHECK(msgrcv(msgQueueId, &msg, sizeof(msg.data), MSG_TYPE_DATA, 0) != -1);
        printf("Sprava c %d: data %s\n",i,msg.data);
    }

    //zrusenie fronty sprav
    CHECK( msgctl(msgQueueId, IPC_RMID, NULL) != -1 );

    exit(EXIT_SUCCESS);
}

void runWriter() {
    int msgQueueId; //id fronty sprav
    MSG_DATA msg; //sprava pre poslanie poctu studentov
    MSG_DATA_COUNT msgCount;

    //ziskanie id ku fronte fronte sprav
    CHECK((msgQueueId = msgget(MSG_QUEUE_KEY, IPC_CREAT | S_IRUSR | S_IWUSR)) != -1);

    msgCount.mtype = MSG_TYPE_COUNT;
    msgCount.count = 2;

    CHECK( msgsnd(msgQueueId,&msgCount,sizeof(int), 0 ) != -1);

    msg.mtype = MSG_TYPE_DATA;
    strncpy(msg.data,"12345",5);

    //poslanie udajov
    CHECK( msgsnd(msgQueueId, &msg, sizeof(msg.data), 0) != -1);
    CHECK( msgsnd(msgQueueId, &msg, sizeof(msg.data), 0) != -1);

}

int main() {
    pid_t pid;

    pid = fork();
    switch(pid) {
        case -1:
            perror("fork");
            break;
        case 0:
            runReader(); //detsky process
            break;
        default:
            runWriter(); //rodicovsky process
            wait(NULL);
    }

    return EXIT_SUCCESS;
}