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
#include <sys/time.h>


int main(int argc, char* argv[]) {
	int producer_id = atoi(argv[1]);

	printf("producer %d: Started\n", producer_id);

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
		int queue = coinToss(0.5);

		sem_wait(mat_lock);
		updateMatrix(matrix_file, producer_id, queue, STATE_WAIT);
		printf("producer %d: Trying to insert in queue %d\n", producer_id, queue);
		sem_post(mat_lock);
		sem_wait(q_lock[queue]);

		sem_wait(mat_lock);
		updateMatrix(matrix_file, producer_id, queue, STATE_LOCK);
		printf("producer %d: Attained lock on queue %d\n", producer_id, queue);
		sem_post(mat_lock);

		struct msqid_ds buf;
		msgctl(q_id[queue], IPC_STAT, &buf);
		// printf("producer %d: buf.msg_qnum = %d\n", producer_id, (int) buf.msg_qnum);
		if((int) buf.msg_qnum >= Q_SIZE) {
			printf("producer %d: Queue %d full\n", producer_id, queue);
		}
		else {
			int val = (rand() % 50) + 1;
			message sentMsg;
			sentMsg.mtype = producer_id + NUM_PC;
			sprintf(sentMsg.mtext, "%d", val);
			if(msgsnd(q_id[queue], &sentMsg, strlen(sentMsg.mtext), 0) == -1) {
				perror("Couldnt send message in queue\n");
			}
			printf("producer %d: Successfully inserted %d in queue %d\n", producer_id, val, queue);
		}


		sem_wait(mat_lock);
		updateMatrix(matrix_file, producer_id, queue, STATE_NONE);
		printf("producer %d: Released lock on queue %d\n", producer_id, queue);
		sem_post(mat_lock);
		sem_post(q_lock[queue]);

		usleep(10000 * (rand() % 5));
	}
}
