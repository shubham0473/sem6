#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <iostream>
#include <queue>

#define MESSAGEQ_KEY 131
#define MESSAGE_SIZE 1000
#define MSG_ADD_ENTRY 1
#define COMPLETED_IO 2

#define WAITING 100
#define TERMINATED 101
#define READY 102
#define RUNNING 103

using namespace std;

struct proc {
    int pid;
    int prio;
    int state;
};

bool operator<(const proc& p1, const proc& p2) {
    return p1.prio > p2.prio;
}

struct message {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
};


std::priority_queue<proc> prio_q;
std::queue<proc> q;
int msgqid;


void init_msqid(){
    if ((msgqid = msgget(MESSAGEQ_KEY, IPC_CREAT | 0666)) < 0) {
        perror("msgget");
        exit(1);
    }
}

proc * round_robin(){

}

proc * prio_round_robin(){

}

int main(int argc, char* argv[]){

    proc new_proc;
    proc *next;

    if(argc < 2){
        cout << "Please provide the scheduling algorithm to use!\n";
        exit(0);
    }

    if(strcmp(argv[1], "RR") == 0) {
        if(msgrcv(msgqid, &response, MESSAGE_SIZE, MSG_ADD_ENTRY, 0) == -1){
            perror("msgrv failed\n");
        }
        if(response.mtext != NULL && response.mtype == MSG_ADD_ENTRY){
            cout << response.mtext << endl;
            new_proc.pid = atoi(response.mtext);
            new_proc.prio = atoi(response.mtext);
            new_proc.state = READY;
            q.push(new_proc);         //change this
        }

		
        next = round_robin();
		// send notify signal to next process
        for(int i = 0; i < 1000; i++){

            //check signal for I/O
            //check signal for temination
            //check message queue for ready processes and preempt if a higher priority exists
			//if any of them true, break out of loop
		}

		//send suspend signal if preempt



    }
    else if(strcmp(argv[1], "PR") == 0){
        if(msgrcv(msgqid, &response, MESSAGE_SIZE, MSG_ADD_ENTRY, 0) == -1){
            perror("msgrv failed\n");
        }
        if(response.mtext != NULL && response.mtype == MSG_ADD_ENTRY){
            cout << response.mtext << endl;
            new_proc.pid = atoi(response.mtext);
            new_proc.prio = atoi(response.mtext);
            prio_q.push(new_proc);         //change this
        }
    }
    else{
        cout << "Given scheduling algorithm doesn't match";
        exit();
    }
    return 0;
}
