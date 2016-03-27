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
#include <iostream>
#include <stdio.h>

using namespace std;

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

void deposit(int msgqid){
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
        memset(&atm_msg, 0, sizeof(Message));
        int status = msgrcv(msgqid, &atm_msg, MESSAGE_SIZE, MTYPE_REPLY, 0);
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

void view(int msgqid){

    Message client_msg;
    strcpy(client_msg.mtext, "VIEW");
    client_msg.mtype = MTYPE_VIEW;
    if(msgsnd(msgqid, &client_msg, strlen(client_msg.mtext), 0) == -1){
        printf("error\n");
    }
    Message atm_msg;
    memset(&atm_msg, 0, sizeof(Message));
    int status = msgrcv(msgqid, &atm_msg, MESSAGE_SIZE, MTYPE_REPLY, 0);
    if(status == -1){
        printf("Error\n");
        exit(0);
    }

    printf("Available balance : %s\n", atm_msg.mtext);

}

void withdraw(int msgqid){

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
        memset(&atm_msg, 0, sizeof(Message));
        int status = msgrcv(msgqid, &atm_msg, MESSAGE_SIZE, MTYPE_REPLY, 0);
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

int entry(int acc_no, int msgqid){
    Message client_msg;
    Message atm_msg;
    memset(&atm_msg, 0, sizeof(Message));
    memset(&client_msg, 0, sizeof(Message));
    printf("%d, %d\n", acc_no, msgqid);
    sprintf(client_msg.mtext, "%d", acc_no);
    client_msg.mtype = MTYPE_ENTER;
    if(msgsnd(msgqid, &client_msg, strlen(client_msg.mtext), 0) == -1){
        printf("error\n");
    }
    int status = msgrcv(msgqid, &atm_msg, MESSAGE_SIZE, MTYPE_REPLY, 0);
    printf("after msgrcv\n");
    if(status == -1){
        printf("msgrcv: Error receiving from atm\n");
        exit(0);
    }
    if(!strcmp(atm_msg.mtext, "OK") && atm_msg.mtype == MTYPE_REPLY){
        printf("hi\n");
        return 1;
    }
    else if(!strcmp(atm_msg.mtext, "ERROR")) return 0;
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
        exit(-1);
    }
    while(fscanf(fp, "%d\t%d\t%s\t%d", &id, &mqid, sem, &shid) != EOF){
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
    char c = temp[5];
    int atm_id = atoi(&c);

    printf("%d\n", atm_id);

    int msgqid, shmid, x;
    char semid[10] = "atm0";

    getID(atm_id, &x, &shmid, semid);
    cout << atm_id << " " << x << " " << shmid << " " << semid << "\n";
    msgqid = init_msqid(x);
    printf("%d\n", msgqid);

    sem_t* lock = sem_open(semid, O_CREAT, O_RDWR, 1);
    sem_wait(lock);
    int status = entry(acc_no, msgqid);
    if(!status){
        sem_post(lock);
        exit(0);
    }

    printf("Options:--\n1. WITHDRAW\n2.DEPOSIT\n3.VIEW\n4.LEAVE\n");
    scanf("%d", &option);

    switch (option) {
        case WITHDRAW : withdraw(msgqid);
        case DEPOSIT : deposit(msgqid);
        case VIEW : view(msgqid);
        case LEAVE : break;
    }

    sem_post(lock);
    return 0;
}
