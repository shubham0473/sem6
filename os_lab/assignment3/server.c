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

typedef struct msgbuf {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;

int client_list[MAX_CLIENTS];
int msgqid;
key_t MESSAGEQ_KEY = 131;

void init_client_list(){
    int i;
    for(i = 0; i < MAX_CLIENTS; i++){
        client_list[i] = DISCONNECTED;
    }
}

void init_msqid(){
    if ((msgqid = msgget(MESSAGEQ_KEY, 0666)) < 0) {
        perror("msgget");
        exit(1);
    }
}

int main()
{
    init_msqid();
    int status, i;

    init_client_list();
    while(1){
        Message client_msg, server_msg;
        for(i = 0; client_list[i] != -1; i++ ){
            printf("Client %d: CONNECTED\n", i);
        }

        status = msgrcv(msgqid, &client_msg, MESSAGE_SIZE, 0, 0);
        printf("%s\n", client_msg.mtext);
        if(client_msg.mtype == 1){
            for(i = 0; client_list[i] != -1; i++);
            client_list[i] = CONNECTED;
            sprintf(server_msg.mtext, "%d", i);
            server_msg.mtype = 3;
            if(msgsnd(msgqid, &server_msg, strlen(server_msg.mtext), 0) == -1){
                printf("error\n");
            }
        }
        else if(client_msg.mtype == 2){
            int temp = atoi(client_msg.mtext);
            printf("Disconnecting client %d\n", temp);
            for(i = 0; client_list[i] != -1; i++){
                if(i == temp) client_list[i] = DISCONNECTED;

            }

        }
        else if(client_msg.mtype == 5){
            server_msg.mtype = 6;
            strcpy(server_msg.mtext, client_msg.mtext);

            for(i = 0; client_list[i] != -1; i++){

                if(msgsnd(msgqid, &server_msg, strlen(server_msg.mtext), 0) == -1){
                    printf("error\n");
                }
            }
        }


    }
    return 1;
}
