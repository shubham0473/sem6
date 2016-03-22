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

#define MESSAGE_SIZE 1000

typedef struct msgbuf {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;



void deposit(){
    printf("ENTER AMOUNT: \n");
    int amount;
    Message client_msg;
    int done = 0;
    do{
        scanf("%d", &amount);
        client_msg.mtype = MTYPE_DEPOSIT;
        sprintf(client_msg.mtext, "%d", amount);
        if(msgsnd(msgqid, &client_msg, strlen(client_msg.mtext), 0) == -1){
            printf("error\n");
        }
        Message atm_msg;
        int status = msgrcv(msgqid, &atm_msg, MESSAGE_SIZE, 0, 0);
        if(status == -1){
            printf("Error\n");
            exit(0);
        }

        if(strcmp(atm_msg.mtext, "OK") == 0){
            printf("SUCCESS\n");
            done = 1;
        }
        else
        {
            printf("%s\n", atm_msg.mtext);
        }
    }while(done != 1);
}

void view(){

    Message client_msg;
    strcpy(client_msg.mtext, "VIEW");
    client_msg.mtype = MTYPE_VIEW;
    if(msgsnd(msgqid, &client_msg, strlen(client_msg.mtext), 0) == -1){
        printf("error\n");
    }
    Message atm_msg;
    int status = msgrcv(msgqid, &atm_msg, MESSAGE_SIZE, 0, 0);
    if(status == -1){
        printf("Error\n");
        exit(0);
    }

    printf("Available balance : %s\n", atm_msg.mtext);

}

void withdraw(int acc_no, int msgqid){

    printf("ENTER AMOUNT: \n");
    int amount;
    Message client_msg;
    int done = 0;
    do{
        scanf("%d", &amount);
        client_msg.mtype = MTYPE_WITHDRAW;
        sprintf(client_msg.mtext, "%d", amount);
        if(msgsnd(msgqid, &client_msg, strlen(client_msg.mtext), 0) == -1){
            printf("error\n");
        }
        Message atm_msg;
        int status = msgrcv(msgqid, &atm_msg, MESSAGE_SIZE, 0, 0);
        if(status == -1){
            printf("Error\n");
            exit(0);
        }

        if(strcmp(atm_msg.mtext, "OK") == 0){
            printf("SUCCESS\n");
            done = 1;
        }
        else
        {
            printf("%s\n", atm_msg.mtext);
        }
    }while(done != 1);

}

void enter(int acc_no, int msgqid){
    Message atm_msg;
    Message master_msg;

    sprintf(atm_msg.mtext, "%d", acc_no);
    if(msgsnd(msgqid, &atm_msg, strlen(atm_msg.mtext), 0) == -1){
        printf("error\n");
    }
    int status = msgrcv(msgqid, &master_msg, MESSAGE_SIZE, 0, 0);

    if(status == -1){
        printf("msgrcv: Error receiving from master\n");
        exit(0);
    }
    if(strcmp(master_msg.mtext, "OK") == 0){

    }

}


void getID(int atm_id, int *msgqid, int *shmid, char semid[]){
    FILE* fp = fopen("ATM_locator.txt", "r");
    int id;
    int mqid;
    int shid;
    char sem[10];
    if(fp == NULL)
    {
        perror("Error: ");
        return(-1);
    }
    for(int i = 0; i < n; i++) {
        fscanf(fp, "%d\t%d\t%s\t%d", &id, &mqid, sem, &shid);
        if(id == atm_id){
            *msgqid = mqid;
            strcpy(semid, sem);
            *shmid = shid;
            break;
        }

    }
    return;
}


int main(){

    int option;
    int atm_no;
    char temp[10];
    int acc_no = getpid();

    printf("Welcome Client\nPlease type 'ENTERx'");
    scanf("%s", temp);
    atm_no = atoi(temp[5]);

    int msgqid, shmid;
    char semid[10];

    getID(&msgqid, &shmid, semid);

    sem_t* lock = sem_open(semid, O_CREAT, O_RDWR, 1);
    sem_wait(lock);

    printf("Options:--\n1. WITHDRAW\n2.DEPOSIT\n3.VIEW\n4.LEAVE\n");
    scanf("%d\n", &option);


    switch (option) {
        case WITHDRAW : withdraw();
        case DEPOSIT : deposit();
        case VIEW : view();
        case LEAVE : break;
    }

    sem_post(lock);
    return 0;
}
