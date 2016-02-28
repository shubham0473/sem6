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

#define DEBUG 0 // 1 if testing, 0 for results

#define SIG_NOTIFY SIGUSR1
#define SIG_SUSPEND SIGUSR2
#define SIG_IO SIGUSR1
#define SIG_TERMINATE SIGUSR2

#define MSG_ADD_ENTRY 1
#define COMPLETED_IO 2

volatile int active = 0;

#define MESSAGE_SIZE 20

struct Message {
    long msgType;
    char msgText[MESSAGE_SIZE+1];
};

key_t MESSAGEQ_KEY = 131;

void notifyHandler(int signum) {
    if(DEBUG) cout << "NOTIFY received" << endl;
    active = 1;
}

void suspendHandler(int signum) {
    if(DEBUG) cout << "SUSPEND received" << endl;
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
    cout << "msgqid received " << endl;
    Message readyMsg;
    readyMsg.msgType = MSG_ADD_ENTRY;
    sprintf(readyMsg.msgText, "%d %d", getpid(), priority);
    if(msgsnd(msgQID, &readyMsg, strlen(readyMsg.msgText), 0) == -1){
        perror("Couldnt send message in queue\n");
    }

    cout << "Pid and priority sent to sched" << endl;
    // Listen for SUSPEND and NOTIFY signals
    signal(SIG_NOTIFY, notifyHandler);
    signal(SIG_SUSPEND, suspendHandler);

    for(int i = 0; i < itr; i++) {
        sleep(1);
		while(!active) {
            pause();
        }

        cout << "PID:" << getpid() << ", Loop:" << i << endl;

        double diceRoll = double(rand())/RAND_MAX;
        if(diceRoll < sleepProb) {
            // Signal IO to sched
			if(DEBUG) cout << "Requesting IO" << endl;
			// Stop listening to NOTIFY and SUSPEND during IO
			// signal(SIG_NOTIFY, SIG_DFL);
		    // signal(SIG_SUSPEND, SIG_DFL);
            kill(schedID, SIG_IO);
            active = 0;

            sleep(sleepTime);

            // Message sched on IO completion
            readyMsg.msgType = COMPLETED_IO;
            if(msgsnd(msgQID, &readyMsg, strlen(readyMsg.msgText), 0) == -1){
                perror("Couldnt send message in queue\n");
            }
			// Listen for SUSPEND and NOTIFY signals
		    // signal(SIG_NOTIFY, notifyHandler);
		    // signal(SIG_SUSPEND, suspendHandler);
			if(DEBUG) cout << "IO complete sent to sched" << endl;
        }
    }
    //signal TERMINATED to sched
    kill(schedID, SIG_TERMINATE);
    cout << "PID:" << getpid() << ", TERMINATED" << endl;

    return 0;
}
