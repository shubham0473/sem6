#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>


#define MESSAGE_SIZE 1000
#define MAX_TRANSACTION_LOGS 100
#define MAX_ACCOUNT 100
#define MAX_ATM 100
#define MTYPE_ENTER 1
#define MTYPE_WITHDRAW 2
#define MTYPE_DEPOSIT 3
#define MTYPE_VIEW 4
#define MTYPE_LEAVE 5
#define MTYPE_SUCCESS 6
#define MTYPE_ERROR 7
#define TRANS_WITHDRAW 1
#define TRANS_DEPOSIT 2
#define WITHDRAW 1
#define DEPOSIT 2
#define VIEW 3
#define LEAVE 4
#define MTYPE_REPLY 8


typedef struct msg {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;


int init_msqid(int key){
    int msgqid = msgget(key, 0666);
    if (msgqid < 0) {
        perror("msgget");
        exit(1);
    }
    return msgqid;
}

int main(){
    int key = 1000;
    int msgqid = init_msqid(key);

    Message message;
    message.mtype = MTYPE_ENTER;
    strcpy(message.mtext, "0");
    if(msgsnd(32769, &message, strlen(message.mtext), 0) == -1){
        printf("error\n");
    }
    return 0;
}
