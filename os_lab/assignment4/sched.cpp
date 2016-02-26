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

#define SIG_NOTIFY SIGUSR1
#define SIG_SUSPEND SIGUSR2
#define SIG_IO SIGUSR1
#define SIG_TERMINATE SIGUSR2

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
    return p1.prio < p2.prio;
}

struct message {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
};


std::priority_queue<proc> prio_q_act;
std::priority_queue<proc> prio_q_exp;
std::queue<proc> q;
int msgqid;
proc *next;
int flag = 0;
int algo_flag = -1;


void init_msqid(){
    if ((msgqid = msgget(MESSAGEQ_KEY, IPC_CREAT | 0666)) < 0) {
        perror("msgget");
        exit(1);
    }
}

void copy_proc(proc *a, proc b){
    a->pid = b.pid;
    a->prio = b.prio;
    a->state = b.state;
}

proc * round_robin(){
    proc *temp;
    temp = (proc*)malloc(sizeof(proc));
    copy_proc(temp, q.front());
    q.pop();
    return temp;
}

proc * prio_round_robin(){
    proc *temp;
    temp = (proc*)malloc(sizeof(proc));
    copy_proc(temp, prio_q_act.top());
    prio_q_exp.push(prio_q_act.top());
    prio_q_act.pop();
    return temp;
}

void handle_IO(int sig){

    if(sig == SIG_IO){
        if(algo_flag == 1){
            flag = 1;
        }
        else if(algo_flag == 2){
            prio_q_exp.push(*next);
            flag = 1;
        }
    }
    cout << "process with pid" << next->pid << " goes for IO" << endl;
}

void handle_termination(int sig){
    if(sig == SIG_TERMINATE){
        if(algo_flag == 1){
            flag = 1;
        }
        else if(algo_flag == 2){
            flag = 1;
        }
    }
    cout << "process with pid" << next->pid << " terminates" << endl;
}


int main(int argc, char* argv[]){

    cout << "sched PID: " << getpid() << endl;

    proc new_proc;
    int count;
    int time_quanta;
    message response;

    init_msqid();

    if(argc < 3){
        cout << "Please provide the scheduling algorithm and time quanta!\n";
        exit(0);
    }

    if(strcmp(argv[1], "RR") == 0) algo_flag = 1;
    else if(strcmp(argv[1], "PR") == 0) algo_flag = 2;
    else{
        cout << "Given scheduling algorithm doesn't match";
        exit(0);
    }

    time_quanta = atoi(argv[2]);

    if(msgrcv(msgqid, &response, MESSAGE_SIZE, MSG_ADD_ENTRY, 0) == -1){
        perror("msgrv failed\n");
    }

    if(response.mtext != NULL && response.mtype == MSG_ADD_ENTRY){
        sscanf(response.mtext, "%d %d", &new_proc.pid, &new_proc.prio);
        printf("%d, %d\n", new_proc.pid, new_proc.prio);
        new_proc.state = READY;
        if(algo_flag == 1) q.push(new_proc);
        else if(algo_flag == 2) prio_q_act.push(new_proc);
    }

    //check signal for I/O
    signal(SIG_IO, handle_IO);
    //check signal for temination
    signal(SIG_TERMINATE, handle_termination);

    do
    {
        flag = 0;
        if(algo_flag == 1) next = round_robin();
        else if(algo_flag == 2) next = prio_round_robin();

        // send notify signal to next process
        kill(next->pid, SIG_NOTIFY);
        cout << "Process with pid " << next->pid << " is running" << endl;
        for(int i = 0; i < time_quanta; i++){
            sleep(1);

            //check message queue for ready processes and preempt if a higher priority exists
            //if any of them true, break out of loop
            msgrcv(msgqid, &response, MESSAGE_SIZE, 0, IPC_NOWAIT);
            if(response.mtext != NULL){
                sscanf(response.mtext, "%d %d", &new_proc.pid, &new_proc.prio);
                new_proc.state = READY;
                if(algo_flag == 1) q.push(new_proc);
                else if(algo_flag == 2) prio_q_act.push(new_proc);
                flag = 1;
                if(response.mtype == COMPLETED_IO){
                    cout << "process with pid " << new_proc.pid << " completes IO" << endl;
                }
            }
            if(flag == 1) break;
        }

        //send suspend signal if preempt
        cout << "Process with pid " << next->pid << " is suspended" << endl;
        kill(next->pid, SIG_SUSPEND);
        if(algo_flag == 1){
            q.push(*next);
        }
        else if(algo_flag == 2){
            prio_q_exp.push(*next);
        }
        if(prio_q_act.empty()){
            std::priority_queue<proc> temp;
            temp = prio_q_act;
            prio_q_act = prio_q_exp;
            prio_q_exp = temp;
        }
        free(next);

    } while(!q.empty() || !prio_q_act.empty());

    //read the result file, calculate average and append
    return 0;
}
