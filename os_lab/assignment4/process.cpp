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
#include <unistd.h>


using namespace std;

#define SIG_NOTIFY SIGUSR1
#define SIG_SUSPEND SIGUSR2
#define SIG_IO SIGUSR1
#define SIG_TERMINATE SIGUSR2

volatile int active = 0;

#define MESSAGE_SIZE 20
struct Message {
    long msgType;
    char msgText[MESSAGE_SIZE+1];
};
key_t MESSAGEQ_KEY = 131;

void notifyHandler(int signum) {
	cout << "NOTIFY received" << endl;
	active = 1;
}

void suspendHandler(int signum) {
	cout << "SUSPEND received" << endl;
	active = 0;
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
	Message readyMsg;
	readyMsg.msgType = 1;
	sprintf(readyMsg.msgText, "%d %d", getpid(), priority);
	if(msgsnd(msgQID, &readyMsg, strlen(readyMsg.msgText), 0) == -1){
		perror("Couldnt send message in queue\n");
	}


	// Listen for SUSPEND and NOTIFY signals
	signal(SIG_NOTIFY, notifyHandler);
	signal(SIG_SUSPEND, suspendHandler);

	for(int i = 0; i < itr; i++) {
		while(!active) {
			pause();
		}

		cout << "PID:" << getpid() << ", Loop:" << i << endl;

		double diceRoll = double(rand())/RAND_MAX;
		if(diceRoll > sleepProb) {
			// Signal IO to sched
			kill(schedID, SIG_IO);
			active = 0;

			sleep(sleepTime);

			// Message sched on IO completion
			readyMsg.msgType = 2;
			if(msgsnd(msgQID, &readyMsg, strlen(readyMsg.msgText), 0) == -1){
				perror("Couldnt send message in queue\n");
			}
		}
	}
	//signal TERMINATED to sched
	kill(schedID, SIG_TERMINATE);
	cout << "PID:" << getpid() << ", TERMINATED" << endl;

	return 0;
}
