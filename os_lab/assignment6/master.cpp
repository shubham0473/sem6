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


#define MTYPE_GLOBALCC 10
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
#define MTYPE_MASTER_VERIFY 20
#define TRANS_WITHDRAW 1
#define TRANS_DEPOSIT 2
#define MTYPE_REPLY 8
#define AVAILABLE 1
#define UNAVAILABLE 0

typedef struct msg {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;

//
// typedef struct ATM{
//     int atmid;
//     int msgqid;
//     int semid;
//     int shmid;
//
// }ATM;
// //
// typedef struct ATM_locator{
//     ATM atm_list[MAX_ATM];
// }ATM_locator;

typedef struct transaction{
    int amount;
    int acc_no;
    int type;
	int avail;
    struct timeval timestamp;

}transaction;

typedef struct account{
    int balance;
    int acc_no;
    struct timeval timestamp;
}account;

typedef struct table{
    account acc_table[MAX_ACCOUNT];
	transaction transaction_log[MAX_TRANSACTION_LOGS];
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
    // cout << key << " " << size << endl;
    int shmid;
    if ((shmid = shmget (key, size, IPC_CREAT | 0666)) == -1)
    {
        perror("shmget: shmget failed");
        exit(1);
    }

    return shmid;
}

void update_atm_locator(FILE *fp, int atmid, int msgqid, int semid, int shmid, int tot_atm){
    // fflush(fp);

    if(fp == NULL)
    {
        perror("Error: ");
        exit(-1);
    }
	// cout << tot_atm << endl;
        char sem[10];
        sprintf(sem, "atm%d", semid);
        fprintf(fp, "%d\t%d\t%s\t%d\n", atmid, msgqid, sem , shmid);          //initialize the ATM_locator file

    // fclose(fp);
    return;
}



int checkAccount(int acc_no, int shmid, int tot_ac){
    account * data = (account*)shmat(shmid, NULL, 0);
    // cout << "in check ac" << endl;
    for(int i = 0; i < tot_ac; i++){
        if(data[i].acc_no == acc_no) {
            return 1;
            // cout << "end of check ac ret 0" << endl;
        }
    }
    int status = shmdt(data);
    if(status == -1){
        printf("Error detaching shm\n");
        exit(0);
    }
    // cout << "end of check ac ret 0" << endl;
    return 0;
}

void globalConsistencyCheck(int shmid, int acc_no, int tot_atm, int tot_ac){
    account * data = (account*)shmat(shmid, NULL, 0);

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
        shmid_x = init_shm(shmid_x, sizeof(table));
		// cout << atmid_x << " " << msgqid_x << " " << semid_x << " " << shmid_x << endl;

        table *temp = (table*)shmat(shmid_x, NULL, 0);
		// int x;

        for(int j = 0; temp->transaction_log[j].avail == UNAVAILABLE && j < MAX_TRANSACTION_LOGS && temp->transaction_log[j].acc_no == acc_no; j++){
			// cout << "enter first loop\n";
            transaction t = temp->transaction_log[j];
            for(int k = 0; k < MAX_ACCOUNT; k++){
				// cout << "enter second loop\n";
                if(data[k].acc_no == acc_no){
					// x = k;
                    if(t.type == TRANS_WITHDRAW){
                        data[k].balance -= t.amount;
                    }
                    else if(t.type == TRANS_DEPOSIT){
                        data[k].balance += t.amount;
                    }
					break;
                }
            }
			temp->transaction_log[j].avail = AVAILABLE;
        }
		// cout << "actual bal: " << temp->acc_table[x].balance << endl;
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
}

int main(int argc, char* argv[]){

    msgctl(atoi(argv[1]), IPC_RMID, 0);
    shmctl(atoi(argv[2]), IPC_RMID, 0);
    int master_msgqid = init_msqid(atoi(argv[1]));
    int shmid = init_shm(atoi(argv[2]), MAX_ACCOUNT*sizeof(account));
    FILE *fp;
    int tot_ac = 0;
    // cout << master_msgqid << " " << shmid << endl;
    int n = atoi(argv[3]);



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

    // cout << "fopen done" << endl;


    for(int i = 0; i < n; i++){
        atm_pid[i] = fork();
        if(atm_pid[i] == 0) {
            // cout << "ENTER CHILD" << endl;
            char args[9][100];
            // sprintf(args[0], "%s", "xterm");
            // sprintf(args[1], "%s", "-hold");
            // sprintf(args[2], "%s", "-e");
            // sprintf(args[3], "%s/./atm", getcwd(NULL, 0));
            sprintf(args[4], "%d", i);
            sprintf(args[5], "%d", master_msgqid);
            sprintf(args[6], "%d", 700+i);
            sprintf(args[7], "%d", 800+i);
            sprintf(args[8], "%d", n);
            execl("atm", "atm", args[4], args[5], args[6], args[7], args[8], NULL);
            perror("Could not start atm: ");

        }
        else
        {
            update_atm_locator(fp, i, 700+i, i, 800+i, n);
        }
    }
    fclose(fp);
    // cout << "ATM CREATED" << endl;
    Message message;
    while(1){
        memset(&message, 0, sizeof(Message));
        // cout << "before msgrcv" << endl;
        // cout << master_msgqid << endl;
        int status = msgrcv(master_msgqid, &message, MESSAGE_SIZE, 0, 0);
        if(status == -1){
            printf("msgrcv: error\n");
            exit(0);
        }
        // cout << "after msgrcv" << endl;
        // cout << message.mtext << endl;

        if(message.mtype == MTYPE_GLOBALCC)                        //either global consistency check
        {
            // cout << "in globalcc" << endl;
            int acc_no = atoi(message.mtext);
            int new_bal = 0;
			printf("master: running global consistency check for ac no %d\n", acc_no);
            globalConsistencyCheck(shmid, acc_no, n, tot_ac);
			printf("master: global consistency check done\n");
            account * data = (account*)shmat(shmid, NULL, 0);
            Message master_msg;
            memset(&master_msg, 0, sizeof(Message));

            for(int i = 0; i < MAX_ACCOUNT; i++){
                if(data[i].acc_no == acc_no) new_bal = data[i].balance;
            }
            sprintf(master_msg.mtext, "%d", new_bal);
            master_msg.mtype = MTYPE_REPLY;
            // cout << "before msggsnd" << endl;
            if(msgsnd(master_msgqid, &master_msg, strlen(master_msg.mtext), 0) == -1){
                printf("error\n");
            }
            int status = shmdt(data);
            if(status == -1){
                printf("Error detaching shm\n");
                exit(0);
            }
        }
        else if(message.mtype == MTYPE_MASTER_VERIFY)                                                        //or new connection request
        {
            // cout << "in master verify" << endl;
            int acc_no = atoi(message.mtext);
			printf("master: verfiying account for acc no %d\n", acc_no);
            int status = checkAccount(acc_no, shmid, tot_ac);
            if(status == 1)
            {
                // cout << "status == 1" << endl;
                Message master_msg;
                memset(&master_msg, 0, sizeof(Message));
                strcpy(master_msg.mtext, "OK");
                master_msg.mtype = MTYPE_REPLY;
                if(msgsnd(master_msgqid, &master_msg, strlen(master_msg.mtext), 0) == -1){
                    printf("error\n");
                }
            }
            else if(status == 0)
            {
                // cout << "status == 0" << endl;
                account * data = (account*)shmat(shmid, NULL, 0);
                data[tot_ac].acc_no = acc_no;
                data[tot_ac].balance = 0;
                gettimeofday(&data[tot_ac].timestamp, NULL);
                tot_ac++;

                Message master_msg;
                memset(&master_msg, 0, sizeof(Message));
                strcpy(master_msg.mtext, "NEW");
                master_msg.mtype = MTYPE_REPLY;
                if(msgsnd(master_msgqid, &master_msg, strlen(master_msg.mtext), 0) == -1){
                    printf("error\n");
                }
                // printf("Message sent\n");
                int status = shmdt(data);
                if(status == -1){
                    printf("Error detaching shm\n");
                    exit(0);
                }
            }
        }
        else{
            cout << message.mtext << " " << message.mtype << endl;
        }
    }



    return 0;
}
