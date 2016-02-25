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


using namespace std;

struct proc {
  int pid;
  int prio;
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

int main(int argc, char* argv[]){

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
      proc = 
    }

  }
  else if(strcmp(argv[1], "PR") == 0){

  }
  else{
    cout << "Given scheduling algorithm doesn't match";
    exit();
  }
  return 0;
}
