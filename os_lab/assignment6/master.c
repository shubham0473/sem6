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


#define QUERY 1
#define REPLY 2
#define MESSAGE_SIZE 1000
#define MAX_TRANSACTION_LOGS 100
#define MAX_ACCOUNT 100
#define MAX_ATM 100

typedef struct msgbuf {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;


// typedef struct ATM{
//     int atmid;
//     int msgqid;
//     int semid;
//     int shmid;
//
// }ATM;
//
// typedef struct ATM_locator{
//     ATM atm_list[MAX_ATM];
// }ATM_locator;

// typedef struct transaction{
//     int amount;
//     struct timeval timestamp;
//
// }transaction;
//
// typedef struct account{
//     int balance;
//     struct timeval timestamp;
// }account;
//
// typedef struct table{
//     transaction trans_log[MAX_TRANSACTION_LOGS];
//     account acc_table[MAX_ACCOUNT];
// }table;

int init_msqid(int key){
    int msgqid = msgget(key, IPC_CREAT | 0666);
    if (msgqid < 0) {
        perror("msgget");
        exit(1);
    }
    return msgqid;
}

int init_shm(key_t key, int size){
    int shmid;
    if ((shmid = shmget (key, size, IPC_CREAT | 0666)) == -1)
    {
        perror("shmget: shmget failed");
        exit(1);
    }
    else
    {
        printf("shmget: shmget returned %d\n", shmid);
        exit(0);
    }

    return shmid;
}

void update_atm_locator(int atm_id, int msgqid, int shmid){

}

int main(int argc, char* argv[]){

    int master_msgqid = init_msqid(atoi(argv[0]));
    int shmid = init_shm(atoi(argv[1]), sizeof(ATM_locator));
    FILE *fp;

    int n = atoi(argv[1]);

    sem_t* atm_sem[n];

    for(int i = 0; i < n; i++){
        char name[10];
        sprintf(name, "atm%d", i);
        sem_unlink(name);
        atm_sem[i] = sem_open(name, O_CREAT, O_RDWR, 1);
    }

    int atm_pid[n];

    fp = fopen("ATM_locator.txt", "w+");
    if(fp == NULL)
    {
        perror("Error: ");
        return(-1);
    }

    for(int i = 0; i < n; i++){
        fprintf(fp, "%d\t%d\tatm%d\t%d\n", 0, 0, i , 0);          //initialize the ATM_locator file
    }

    for(int i = 0; i < n; i++){
        atm_pid[i] = fork();
        if(atm_pid[i] == 0) {
            char args[8][100];
            sprintf(args[0], "%s", "xterm");
            sprintf(args[1], "%s", "-hold");
            sprintf(args[2], "%s", "-e");
            sprintf(args[3], "%s/./atm", getcwd(NULL, 0));
            sprintf(args[4], "%d", i);
            sprintf(args[5], "%d", master_msgqid);
            sprintf(args[6], "%d", i);
            sprintf(args[7]), "%d", i);
            execl("/usr/bin/xterm", args[0], args[1], args[2], args[3], args[4], args[5],args[6], args[7], NULL);
            perror("Could not start train: ");

        }
        else
        {
            update_atm_locator(i, i, i);
        }
    }






    return 0;
}
