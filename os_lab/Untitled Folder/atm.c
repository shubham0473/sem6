#include <stdio.h>
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
#include <sys/shm.h>

#define MESSAGE_SIZE 1000
#define MAX_TRANSACTION_LOGS 100
#define MAX_ACCOUNT 100
#define MTYPE_ENTER 1
#define MTYPE_WITHDRAW 2
#define MTYPE_DEPOSIT 3
#define MTYPE_VIEW 4
#define MTYPE_LEAVE 5
#define MTYPE_SUCCESS 6
#define MTYPE_ERROR 7
#define MTYPE_MASTER_VERIFY 1


typedef struct msgbuf {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;

typedef struct transaction{
    int amount;
    struct timeval timestamp;

}transaction;

typedef struct account{
    int balance;
    struct timeval timestamp;
}account;

typedef struct table{
    transaction trans_log[MAX_TRANSACTION_LOGS];
    account acc_table[MAX_ACCOUNT];
}table;


int init_msqid(int key){
    int msgqid = msgget(key, IPC_CREAT | 0666);
    if (msgqid < 0) {
        perror("msgget");
        exit(1);
    }
    return msgqid;
}

int init_shm(key_t key, int size){
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget: shmget failed");
        exit(1);
    }
    else
    {
        (void) fprintf(stderr, "shmget: shmget returned %d\n", shmid);
        exit(0);
    }

    return shmid;
}


void withdraw(){
    localConsistencyCheck();

}

void deposit(){

}

void view(){
    globalConsistencyCheck();
}

int main(int argc, char* argv[]){

    if(argc < 5){
        printf("Usage : atm <msgqid> <shmid> <master_msgqid> \n");
        exit(0);
    }

    int atm_id = atoi(argv[1]);
    int master_msgqid = atoi(argv[2]);
    int msgqid = init_msqid(atoi(argv[3]));
    int shmid = init_shm(atoi(argv[4]), sizeof(table));


    table* data = shmat(shmid, NULL, 0);

    Message message;
    while(1){
        int status = msgrcv(msgqid, &message, MESSAGE_SIZE, 0, 0);
        if(status == -1){
            printf("msgrcv: error\n");
            exit(0);
        }

        printf("%s\n", message.mtext);

        if(message.mtype == MTYPE_ENTER)
        {
            //VERIFY account
            Message atm_msg;
            atm_msg.mtype = MTYPE_MASTER_VERIFY;
            strcpy(atm_msg.mtext, message.mtext);
            if(msgsnd(msgqid, &atm_msg, strlen(atm_msg.mtext), 0) == -1){
                printf("error\n");
            }
            Message master_msg;
            status = msgrcv(master_msgqid, &master_msg, MESSAGE_SIZE, 0, 0);
            if(status == -1){
                printf("msgrcv: error\n");
                exit(0);
            }
            printf("atm: notified master\n");

        }

        switch (message.mtype) {
            case MTYPE_WITHDRAW : withdraw();
            case MTYPE_DEPOSIT : deposit();
            case MTYPE_VIEW : view();
            default: break;
        }
    }

    return 0;
}
