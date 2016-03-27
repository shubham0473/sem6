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
#include <deque>
#include <iostream>

using namespace std;

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
#define MTYPE_MASTER_VERIFY 20
#define TRANS_WITHDRAW 1
#define TRANS_DEPOSIT 2
#define MTYPE_REPLY 8
#define MTYPE_GLOBALCC 10

typedef struct msg {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;

typedef struct transaction{
    int amount;
    int acc_no;
    int type;
    struct timeval timestamp;

}transaction;

typedef struct account{
    int balance;
    int acc_no;
    struct timeval timestamp;
}account;

typedef struct table{
    deque<transaction> transaction_log;
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

int init_shm(key_t key, size_t size){
    cout << key << " " << size << endl;
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget: shmget failed");
        exit(1);
    }
    else
    {
        (void) fprintf(stderr, "shmget: shmget returned %d\n", shmid);
    }

    return shmid;
}


int localConsistencyCheck(int shmid, int amt, int acc_no, int tot_ac, int tot_atm){
    int current_balance;
    table* data = (table*)shmat(shmid, NULL, 0);
    for(int i = 0; i < tot_ac; i++){
        if(data->acc_table[i].acc_no == acc_no){
            current_balance = data->acc_table[i].balance;
        }
    }

    FILE * fp;
    fp = fopen("ATM_locator.txt", "r");
    if(fp == NULL){
        printf("Error: fopen\n");
        exit(0);
    }

    for(int i = 0; i < tot_atm; i++){
        int atmid_x;
        int msgqid_x;
        char semid_x[10];
        int shmid_x;            // shmid on atm x
        fscanf(fp, "%d\t%d\t%s\t%d", &atmid_x, &msgqid_x, semid_x, &shmid_x);
        shmid_x = init_shm(shmid_x, (sizeof(table)+sizeof(transaction)*MAX_TRANSACTION_LOGS));
        table *temp = (table*)shmat(shmid_x, NULL, 0);

        for(int j = 0; !temp->transaction_log.empty(); j++){
            if(temp->transaction_log[j].acc_no == acc_no){
                if(temp->transaction_log[j].type == TRANS_WITHDRAW) current_balance -= temp->transaction_log[j].amount;
                else if(temp->transaction_log[j].type == TRANS_DEPOSIT) current_balance += temp->transaction_log[j].amount;
            }
        }
        int status = shmdt(temp);
        if(status == -1){
            printf("Error Detaching\n");
            exit(0);
        }

    }
    int status = shmdt(data);
    if(status == -1){
        printf("Error detaching shm\n");
        exit(0);
    }
    if(amt > current_balance) return -1;
    else return 1;
}

void withdraw(int shmid, int amt, int acc_no, int tot_ac, int tot_atm, int msgqid){
    table* data = (table*)shmat(shmid, NULL, 0);
    int valid = localConsistencyCheck(shmid, amt, acc_no, tot_ac, tot_atm);
    Message atm_msg;
    memset(&atm_msg, 0, sizeof(Message));
    // memset(atm_msg, 0, sizeof(atm_msg));
    if(valid == -1){
        strcpy(atm_msg.mtext, "ERROR");
        atm_msg.mtype == MTYPE_REPLY;

    }
    else if(valid == 1){
        strcpy(atm_msg.mtext, "OK");
        atm_msg.mtype == MTYPE_REPLY;
        transaction t;
        t.amount = amt;
        t.acc_no = acc_no;
        t.type = TRANS_WITHDRAW;
        gettimeofday(&t.timestamp,NULL);

        data->transaction_log.push_back(t);

    }

    int status = msgsnd(msgqid, &atm_msg, strlen(atm_msg.mtext), 0);
    if(status == -1){
        printf("Error msgsnd\n");
        exit(0);
    }

    int ret = shmdt(data);
    if(ret == -1){
        printf("Error detaching shm\n");
        exit(0);
    }

    return;
}

void deposit(int shmid, int amt, int acc_no, int tot_ac, int msgqid){

    table* data = (table*)shmat(shmid, NULL, 0);

    transaction t;
    t.amount = amt;
    t.acc_no = acc_no;
    t.type = TRANS_DEPOSIT;
    gettimeofday(&t.timestamp,NULL);

    data->transaction_log.push_back(t);

    Message atm_msg;
    memset(&atm_msg, 0, sizeof(Message));
    // memset(atm_msg, 0, sizeof(atm_msg));
    strcpy(atm_msg.mtext, "OK");
    atm_msg.mtype = MTYPE_REPLY;
    int status = msgsnd(msgqid, &atm_msg, strlen(atm_msg.mtext), 0);
    if(status == -1){
        printf("Error msgsnd\n");
        exit(0);
    }
    int ret = shmdt(data);
    if(ret == -1){
        printf("Error detaching shm\n");
        exit(0);
    }
    return;

}

void view(int shmid, int acc_no, int master_msgqid){
    Message atm_msg;
    memset(&atm_msg, 0, sizeof(Message));
    // memset(atm_msg, 0, sizeof(atm_msg));
    atm_msg.mtype = MTYPE_GLOBALCC;
    sprintf(atm_msg.mtext, "%d", acc_no);
    int status = msgsnd(master_msgqid, &atm_msg, strlen(atm_msg.mtext), 0);
    if(status == -1){
        printf("Error msgsnd\n");
        exit(0);
    }

}

void enter(int shmid, int acc_no, int master_msgqid, int tot_ac, int msgqid){
    cout << "in mtye enter" << endl;
    table* data = (table*)shmat(shmid, NULL, 0);
    //VERIFY account
    Message atm_msg;
    memset(&atm_msg, 0, sizeof(Message));
    atm_msg.mtype = MTYPE_MASTER_VERIFY;
    // cl_ac_no = atoi(message.mtext);
    // strcpy(atm_msg.mtext, message.mtext);
    sprintf(atm_msg.mtext, "%d", acc_no);
    atm_msg.mtype = MTYPE_MASTER_VERIFY;
    if(msgsnd(master_msgqid, &atm_msg, strlen(atm_msg.mtext), 0) == -1){
        printf("error\n");
    }
    Message master_msg;
    memset(&master_msg, 0, sizeof(Message));
    int status = msgrcv(master_msgqid, &master_msg, MESSAGE_SIZE, MTYPE_REPLY, 0);
    if(status == -1){
        printf("msgrcv: error\n");
        exit(0);
    }
    printf("atm: notified master\n");
    printf("%s\n", master_msg.mtext);
    if(strcmp(master_msg.mtext, "NEW") == 0){
        data->acc_table[tot_ac].acc_no = acc_no;
        data->acc_table[tot_ac].balance = 0;
        gettimeofday(&data->acc_table[tot_ac].timestamp, NULL);
        tot_ac++;
    }
    int ret = shmdt(data);
    if(ret == -1){
        printf("Error detaching shm\n");
        exit(0);
    }
    Message reply;
    memset(&reply, 0, sizeof(Message));
    reply.mtype == MTYPE_REPLY;
    strcpy(reply.mtext, "OK");
    if(msgsnd(msgqid, &reply, strlen(reply.mtext), 0) == -1){
        perror("msgsnd");
    }
}

int main(int argc, char* argv[]){

    if(argc < 5){
        printf("Usage : atm <master_msgqid> <shmid> <msgqid> \n");
        exit(0);
    }
    msgctl(atoi(argv[3]), IPC_RMID, 0);
    shmctl(atoi(argv[4]), IPC_RMID, 0);


    int atm_id = atoi(argv[1]);
    int master_msgqid = atoi(argv[2]);
    int msgqid = init_msqid(atoi(argv[3]));
    int shmid = init_shm(atoi(argv[4]), (sizeof(table)+sizeof(transaction)*MAX_TRANSACTION_LOGS));
    int tot_atm = atoi(argv[5]);


    int cl_ac_no = 0;
    int tot_ac = 0;

    cout << msgqid << " " << shmid << endl;

    Message message;
    while(1){
        cout << "before msgrcv" << endl;
        memset(&message, 0, sizeof(Message));
        int status = msgrcv(msgqid, &message, MESSAGE_SIZE, 0, 0);
        if(status == -1){
            printf("msgrcv: error\n");
            exit(0);
        }
        cout << "after msgrcv" << endl;

        printf("%s\n", message.mtext);

        switch (message.mtype) {
            case MTYPE_ENTER :
            cl_ac_no = atoi(message.mtext);
            enter(shmid, cl_ac_no, master_msgqid, tot_ac, msgqid);
            case MTYPE_WITHDRAW : withdraw(shmid, atoi(message.mtext), cl_ac_no, tot_ac, tot_atm, msgqid);
            case MTYPE_DEPOSIT : deposit(shmid, atoi(message.mtext), cl_ac_no, tot_ac, msgqid);
            case MTYPE_VIEW : view(shmid, cl_ac_no, master_msgqid);
        }
    }

    return 0;
}
