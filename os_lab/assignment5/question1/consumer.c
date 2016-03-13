#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ass5.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>

int num_consumes;

// Return number of consumes by this consumer when killed by manager
void terminateHandler(int SIG) {
	exit(num_consumes);
}

int main(int argc, char* argv[]) {
	int consumer_id = atoi(argv[1]);
	double p = atof(argv[2]);
	int avoid = atoi(argv[3]);
	num_consumes = 0;

	signal(SIGUSR1, terminateHandler);


	printf("consumer %d: Started\n", consumer_id);

	sem_t* mat_lock = sem_open("mat_lock", O_CREAT, O_RDWR, 0);
	FILE* matrix_file = fopen("matrix.txt", "r+");

	sem_t* q_lock[NUM_Q];
	q_lock[0] = sem_open("q_lock_0", O_CREAT, O_RDWR, 1);
	q_lock[1] = sem_open("q_lock_1", O_CREAT, O_RDWR, 1);

	int q_id[NUM_Q];
	q_id[0] = msgget(Q1_KEY, 0666);
	q_id[1] = msgget(Q2_KEY, 0666);

	srand(time(NULL));

	while(1) {
		int bothQueues = coinToss(1 - p);
		int queue = (bothQueues && avoid) ? 0 : coinToss(0.5);
		int queue_p = 1 - queue;

		sem_wait(mat_lock);
		updateMatrix(matrix_file, consumer_id, queue, STATE_WAIT);
		printf("consumer %d: Trying to consume from queue %d\n", consumer_id, queue);
		sem_post(mat_lock);
		sem_wait(q_lock[queue]);

		sem_wait(mat_lock);
		updateMatrix(matrix_file, consumer_id, queue, STATE_LOCK);
		printf("consumer %d: Attained lock on queue %d\n", consumer_id, queue);
		sem_post(mat_lock);

		struct msqid_ds buf;
		msgctl(q_id[queue], IPC_STAT, &buf);
		// printf("consumer %d: buf.msg_qnum = %d\n", consumer_id, (int) buf.msg_qnum);
		if((int) buf.msg_qnum == 0) {
			printf("consumer %d: Queue %d empty\n", consumer_id, queue);
		}
		else {
			message recvdMsg;
			if(msgrcv(q_id[queue], &recvdMsg, 5, 0, IPC_NOWAIT) == -1){
				printf("consumer %d:", consumer_id);
				perror(" Couldnt receive message in queue\n");
				exit(0);
			}
			int val = atoi(recvdMsg.mtext);
			printf("consumer %d: consume no:%d queue:%d value:%d producer:%ld\n", consumer_id, num_consumes++, queue, val, recvdMsg.mtype - NUM_PC);
		}

		if(bothQueues) {
			sem_wait(mat_lock);
			updateMatrix(matrix_file, consumer_id, queue_p, STATE_WAIT);
			printf("consumer %d: Trying to consume from queue_p %d\n", consumer_id, queue_p);
			sem_post(mat_lock);
			sem_wait(q_lock[queue_p]);

			sem_wait(mat_lock);
			updateMatrix(matrix_file, consumer_id, queue_p, STATE_LOCK);
			printf("consumer %d: Attained lock on queue %d\n", consumer_id, queue);
			sem_post(mat_lock);

			struct msqid_ds buf;
			msgctl(q_id[queue_p], IPC_STAT, &buf);
			// printf("consumer %d: buf.msg_qnum = %d\n", consumer_id, (int) buf.msg_qnum);
			if((int) buf.msg_qnum == 0) {
				printf("consumer %d: Queue %d empty\n", consumer_id, queue);
			}
			else {
				message recvdMsg;
				if(msgrcv(q_id[queue_p], &recvdMsg, 5, 0, IPC_NOWAIT) == -1){
					printf("consumer %d:", consumer_id);
					perror(" Couldnt receive message in queue_p\n");
					exit(0);
				}
				int val = atoi(recvdMsg.mtext);
				printf("consumer %d: consume no:%d queue_p:%d value:%d producer:%ld\n", consumer_id, num_consumes++, queue_p, val, recvdMsg.mtype - NUM_PC);
			}
		}

		sem_wait(mat_lock);
		updateMatrix(matrix_file, consumer_id, queue, STATE_NONE);
		printf("consumer %d: Released lock on queue %d\n", consumer_id, queue);
		sem_post(mat_lock);
		sem_post(q_lock[queue]);

		if(bothQueues) {
			sem_wait(mat_lock);
			updateMatrix(matrix_file, consumer_id, queue_p, STATE_NONE);
			printf("consumer %d: Released lock on queue_p %d\n", consumer_id, queue_p);
			sem_post(mat_lock);
			sem_post(q_lock[queue_p]);
		}

		usleep(1 * (rand() % 10 + 1));
	}
}
