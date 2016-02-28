#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>
#include <cstdio>

#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <unistd.h>


using namespace std;

#define DEBUG 1 // 1 if testing, 0 for results

#define SIG_NOTIFY SIGUSR1
#define SIG_SUSPEND SIGUSR2
#define SIG_IO SIGUSR1
#define SIG_TERMINATE SIGUSR2
#define MIL 1000000

#define MSG_ADD_ENTRY 1
#define COMPLETED_IO 2

volatile int active = 0;
struct timeval start_time;
struct timeval end_time;
volatile long long response_time, waiting_time, tat;

#define MESSAGE_SIZE 20

struct Message {
	long msgType;
	char msgText[MESSAGE_SIZE+1];
};

key_t MESSAGEQ_KEY = 131;

void notifyHandler(int signum) {
	if(DEBUG) cout << "NOTIFY received" << endl;
	active = 1;
	gettimeofday(&start_time, NULL);
	long long temp_start = start_time.tv_sec * MIL + start_time.tv_usec;
	long long temp_end = end_time.tv_sec * MIL + end_time.tv_usec;
	waiting_time += temp_start - temp_end;
}

void suspendHandler(int signum) {
	if(DEBUG) cout << "SUSPEND received" << endl;
	active = 0;
	gettimeofday(&end_time, NULL);
}

int main(int argc, char* argv[]) {
	assert(argc > 4);
	int itr = atoi(argv[1]);
	int priority = atoi(argv[2]);
	double sleepProb = atof(argv[3]);
	int sleepTime = atoi(argv[4]);
	int schedID = atoi(argv[5]);

	srand(time(NULL));

	// Send pid and priority to scheduler in msg queue
	int msgQID;
	if ((msgQID = msgget(MESSAGEQ_KEY, 0666)) < 0) {
		perror("msgget failed\n");
		exit(1);
	}
	cout << "msgqid received " << endl;

	// Listen for SUSPEND and NOTIFY signals
	signal(SIG_NOTIFY, notifyHandler);
	signal(SIG_SUSPEND, suspendHandler);

	Message readyMsg;
	readyMsg.msgType = MSG_ADD_ENTRY;
	sprintf(readyMsg.msgText, "%d %d", getpid(), priority);
	if(msgsnd(msgQID, &readyMsg, strlen(readyMsg.msgText), 0) == -1){
		perror("Couldnt send message in queue\n");
	}
	cout << "Pid and priority sent to sched" << endl;

	gettimeofday(&start_time, NULL);
	end_time = start_time;
	long long temp_start = start_time.tv_sec * MIL + start_time.tv_usec;
	response_time = temp_start, waiting_time = 0, tat = temp_start;

	for(int i = 0; i < itr; i++) {
		usleep(100000);
		while(!active) {
			pause();
		}
		if(i == 0) {
			struct timeval temp_time;
			gettimeofday(&temp_time, NULL);
			long long temp_temp = temp_time.tv_sec * MIL + temp_time.tv_usec;
			response_time = temp_temp - response_time;
		}

		cout << "PID:" << getpid() << ", Loop:" << i << endl;

		double diceRoll = double(rand())/RAND_MAX;
		if(diceRoll < sleepProb) {
			// Signal IO to sched
			cout << "Requesting IO" << endl;
			// Stop listening to NOTIFY and SUSPEND during IO
			// signal(SIG_NOTIFY, SIG_DFL);
			// signal(SIG_SUSPEND, SIG_DFL);
			kill(schedID, SIG_IO);
			active = 0;

			sleep(sleepTime);

			// Message sched on IO completion
			if(i < itr - 1) {
				readyMsg.msgType = COMPLETED_IO;
				if(msgsnd(msgQID, &readyMsg, strlen(readyMsg.msgText), 0) == -1){
					perror("Couldnt send message in queue\n");
				}
			}
			gettimeofday(&end_time, NULL);
			// Listen for SUSPEND and NOTIFY signals
			// signal(SIG_NOTIFY, notifyHandler);
			// signal(SIG_SUSPEND, suspendHandler);
			cout << "IO complete" << endl;
		}
	}
	//signal TERMINATED to sched
	kill(schedID, SIG_TERMINATE);
	cout << "PID:" << getpid() << ", TERMINATED" << endl;

	struct timeval temp_time;
	gettimeofday(&temp_time, NULL);
	long long temp_temp = temp_time.tv_sec * MIL + temp_time.tv_usec;
	tat = temp_temp - tat;

	FILE* fp = fopen("results.out", "a");
	fprintf(fp, "PID: %d, Priority: %d, Response time: %.4fs, Waiting time: %.4fs, Turnaround time: %.4fs\n", getpid(), priority, double(response_time)/MIL, double(waiting_time)/MIL, double(tat)/MIL);
	fclose(fp);

	return 0;
}
