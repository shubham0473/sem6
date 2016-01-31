#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define MAX_CLIENTS 100
#define MESSAGE_SIZE 3000
#define CONNECTED 1
#define DISCONNECTED -1


int client_list[MAX_CLIENTS];
int msgqid;
key_t MESSAGEQ_KEY = 131;

void init_client_list(){
	int i;
	for(i = 0; i < MAX_CLIENTS; i++){
		client_list[i] = DISCONNECTED;
	}
}

void init_msgq(){

}

int main()
{
	int msgid,len;
	key_t key;
	key=131;
	msgid=msgget(key,IPC_CREAT|0666);
	printf("\nq=%d",msgid);
}
