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

#define ROUND_ROBIN 1
#define PRIORITY_RR 2

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
std::deque<proc> ready_q;
int wait_q = 0;
int total_proc = 0;
int msgqid;
proc *next;
int IO_flag = 0, terminated_flag = 0;
int algo_flag = -1;


void init_msqid(){
	if ((msgqid = msgget(MESSAGEQ_KEY, IPC_CREAT | 0666)) < 0) {
		perror("msgget");
		exit(1);
	}
}

void check_msgq() {

}

void copy_proc(proc *a, proc b){
	a->pid = b.pid;
	a->prio = b.prio;
	a->state = b.state;
}

proc * round_robin(){
	if(ready_q.empty()) return NULL;
	proc* temp = (proc*) malloc(sizeof(proc));
	copy_proc(temp, ready_q.front());
	ready_q.pop_front();
	return temp;
}

proc * prio_round_robin(){
	if(prio_q_act.empty()) return NULL;
	proc* temp = (proc*) malloc(sizeof(proc));
	copy_proc(temp, prio_q_act.top());
	// prio_q_exp.push(prio_q_act.top());
	prio_q_act.pop();
	return temp;
}

void handle_IO(int sig){
	if(sig == SIG_IO){
		if(algo_flag == ROUND_ROBIN){
			next->state = WAITING;
			wait_q++;
			IO_flag = 1;
		}
		else if(algo_flag == PRIORITY_RR){
			next->state = WAITING;
			wait_q++;
			IO_flag = 1;
		}
	}
	cout << "pid: " << next->pid << " goes for IO" << endl;
}

void handle_termination(int sig){
	if(sig == SIG_TERMINATE){
		if(algo_flag == ROUND_ROBIN){
			next->state = TERMINATED;
			terminated_flag = 1;
		}
		else if(algo_flag == PRIORITY_RR){
			next->state = TERMINATED;
			terminated_flag = 1;
		}
	}
	cout << "pid: " << next->pid << " terminates" << endl;
	total_proc--;
}


int main(int argc, char* argv[]){

	cout << "sched PID: " << getpid() << endl;

	int count;
	int time_quanta;

	init_msqid();

	if(argc < 3){
		cout << "ERROR! Insufficient arguments!\nUsage: ./sched <RR/PR> <time_quanta>\n";
		exit(0);
	}

	if(strcmp(argv[1], "RR") == 0) algo_flag = ROUND_ROBIN;
	else if(strcmp(argv[1], "PR") == 0) algo_flag = PRIORITY_RR;
	else {
		cout << "Given scheduling algorithm doesn't match";
		exit(0);
	}

	time_quanta = atoi(argv[2]);

	message response;
	if(msgrcv(msgqid, &response, MESSAGE_SIZE, MSG_ADD_ENTRY, 0) == -1) { //blocking for the first process
		perror("msgrv failed\n");
	}
	if(response.mtext != NULL && response.mtype == MSG_ADD_ENTRY){
		proc new_proc;
		sscanf(response.mtext, "%d %d", &new_proc.pid, &new_proc.prio);
		printf("New process: %d, Priority: %d added\n", new_proc.pid, new_proc.prio);
		total_proc++;
		new_proc.state = READY;
		if(algo_flag == ROUND_ROBIN) ready_q.push_back(new_proc);
		else if(algo_flag == PRIORITY_RR) prio_q_act.push(new_proc);
	}

	//check signal for I/O
	signal(SIG_IO, handle_IO);
	//check signal for temination
	signal(SIG_TERMINATE, handle_termination);


	do {
		IO_flag = 0;
		terminated_flag = 0;
		cout << "QUEUE: Total_proc = " << total_proc <<  endl;
		for(int i = 0; i < ready_q.size(); i++) {
			proc p = ready_q[i];
			cout << "<pid>: " << p.pid << ", <prio>: " << p.prio << ", <state>: " << p.state << endl;
		}
		//check message queue for ready processes and preempt if a higher priority exists in PRR
		message response;
		int param  = (ready_q.empty() && prio_q_act.empty()) ? 0 : IPC_NOWAIT;
		while(msgrcv(msgqid, &response, MESSAGE_SIZE, 0, param) != -1) {
			if(response.mtext != NULL) {
				proc new_proc;
				sscanf(response.mtext, "%d %d", &new_proc.pid, &new_proc.prio);
				new_proc.state = READY;
				if(algo_flag == ROUND_ROBIN) ready_q.push_back(new_proc);
				else if(algo_flag == PRIORITY_RR) {
					prio_q_act.push(new_proc);
				}
				if(response.mtype == COMPLETED_IO){
					cout << "pid: " << new_proc.pid << " completes IO" << endl;
					wait_q--;
				}
				else if(response.mtype == MSG_ADD_ENTRY) {
					total_proc++;
					printf("New process: %d, Priority: %d added\n", new_proc.pid, new_proc.prio);
				}
			}
			param = (ready_q.empty() && prio_q_act.empty()) ? 0 : IPC_NOWAIT;
		}
		if(algo_flag == ROUND_ROBIN) next = round_robin();
		else if(algo_flag == PRIORITY_RR) next = prio_round_robin();
		if(next == NULL || next-> state != READY) continue;
		next->state = RUNNING;

		// send notify signal to next process
		kill(next->pid, SIG_NOTIFY);
		cout << "pid: " << next->pid << " is running" << endl;
		int i;
		for(i = 0; i < time_quanta; i++){
			sleep(1);
			if(IO_flag || terminated_flag) break;
		}

		//send suspend signal on preempt (beacause of time quanta exp or preempt)
		if(i == time_quanta) {
			cout << "pid: " << next->pid << " is suspended" << endl;
			kill(next->pid, SIG_SUSPEND);
		}
		if(next->state == RUNNING) {
			next->state = READY;
			if(algo_flag == ROUND_ROBIN){
				ready_q.push_back(*next);
			}
			else if(algo_flag == PRIORITY_RR){
				prio_q_exp.push(*next);
			}
		}

		if(prio_q_act.empty()){
			std::priority_queue<proc> temp;
			temp = prio_q_act;
			prio_q_act = prio_q_exp;
			prio_q_exp = temp;
		}
		free(next);
	} while(total_proc > 0) ;
	//while(!ready_q.empty() || !prio_q_act.empty() || wait_q > 0);
	//TODO: read the result file, calculate average and append
	return 0;
}
