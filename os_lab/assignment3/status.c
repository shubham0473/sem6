#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <mqueue.h>

int main(){
	int qid;
	struct msqid_ds qstat;
	qid=msgget((key_t)131,IPC_CREAT);
	if(qid==-1)
	{
		perror("msg failed\n");
		exit(1);
	}
	if(msgctl(qid,IPC_STAT,&qstat)<0)
	{
		perror("msgctl failed");
		exit(1);
	}
	printf("\n%d msg in q",qstat.msg_qnum);
	printf("last msg send by process %d",qstat.msg_lspid);
	printf("last msg receved by process %d",qstat.msg_lrpid);
	printf("current number of bytes on queue %d",qstat.msg_cbytes);
	printf("max number of bytes %d",qstat.msg_qbytes);
    }
