/*******************************************
*
*	Assignment 5, OS Lab, Spring 2016
*
*	Group 10:
*	Divyansh Gupta (13CS30011)
*	Shubham Agrawal (13CS30030)
*
*	Solutions (with and without deadlocks) to
*	to the Producer Consumer Problem with
*	5 producers, 5 consumers and 2 buffers.
*
*	Uses POSIX names semaphores and System V
*	message queus.
*******************************************/
#include "ass5.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>

int main(int argc, char* argv[]) {

	if(argc < 2) {
		perror("Usage: manager <p value> <deadlock avoidance>");
		return -1;
	}

	// Initialize matrix file
	int producer_pid[NUM_PC], consumer_pid[NUM_PC];
	int producer_states[NUM_PC][NUM_Q], consumer_states[NUM_PC][NUM_Q];
	for(int i = 0; i < NUM_Q; i++) {
		for(int j = 0; j < NUM_PC; j++) {
			producer_states[j][i] = 0;
		}
		for(int j = 0; j < NUM_PC; j++) {
			consumer_states[j][i] = 0;
		}
	}
	sem_unlink("mat_lock"); // Remove previously existing semaphore
	sem_t* mat_lock = sem_open("mat_lock", O_CREAT, O_RDWR, 1);
	if(mat_lock == SEM_FAILED) {
		perror("Could not start semaphore\n");
		return 0;
	}
	sem_wait(mat_lock);
	FILE* matrix_file = fopen("matrix.txt", "w+");
	writeMatrix(matrix_file, producer_states, consumer_states);
	fflush(matrix_file);
	readMatrix(matrix_file, producer_states, consumer_states);
	printf("manager: Matrix file initialized to:\n");
	writeMatrix(stdout, producer_states, consumer_states);
	fclose(matrix_file);
	sem_post(mat_lock);

	// Create queue semaphores
	sem_t* q_lock[NUM_Q];
	// for(int i = 0; i < NUM_Q; i++) {
	// 	char sem_name[10];
	// 	sprintf(sem_name, "q_lock_%d", i);
	// 	sem_unlink(sem_name); // Remove sem if already existing
	// 	q_lock[i] = sem_open(sem_name, O_CREAT, O_RDWR, 1);
	// }
	sem_unlink("q_lock_0"); // Remove sem if already existing
	q_lock[0] = sem_open("q_lock_0", O_CREAT, O_RDWR, 1);
	sem_unlink("q_lock_1");
	q_lock[1] = sem_open("q_lock_1", O_CREAT, O_RDWR, 1);

	// Create message queues
	int q_id[NUM_Q];
	for(int i = 0; i < NUM_Q; i++) {
		q_id[i] = msgget(Q_BASE_KEY + i, 0666);
		msgctl(q_id[i], IPC_RMID, NULL); // Remove Q if already existing
		q_id[i] = msgget(Q_BASE_KEY + i, 0666 | IPC_CREAT);
	}

	for(int i = 0; i < NUM_PC; i++){
		producer_pid[i] = fork();
		if(producer_pid[i] == 0) {
			char* args[3];
			args[0] = "producer";
			sprintf(args[1], "%d", i);
			args[2] = NULL;
			execv(args[0], args);
			perror("Could not start producer: "); // exec shouldnt return
		}
	}

	for(int i = 0; i < NUM_PC; i++){
		consumer_pid[i] = fork();
		if(consumer_pid[i] == 0) {
			char* args[5];
			args[0] = "consumer";
			sprintf(args[1], "%d", i + NUM_PC);
			args[2] = (char*) malloc(sizeof(char) * 10);
			strcpy(args[2], argv[1]);
			args[3] = (char*) malloc(sizeof(char) * 10);
			strcpy(args[3], argv[2]);
			args[4] = NULL;
			execv(args[0], args);
			perror("Could not start consumer: ");
		}
	}

	while(1) {
		sleep(2);

		sem_wait(mat_lock);
		matrix_file = fopen("matrix.txt", "r+");
		fflush(matrix_file);
		readMatrix(matrix_file, producer_states, consumer_states);
		writeMatrix(stdout, producer_states, consumer_states);
		fclose(matrix_file);
		sem_post(mat_lock);

		int cycle[4];
		if(checkCycle(consumer_states, cycle) == 1) {
			printf("manager: Deadlock detected!\n");
			printCycle(cycle);
			fflush(stdout);


			for(int i = 0; i < NUM_Q; i++) {
				sem_close(q_lock[i]);

				q_id[i] = msgget(Q_BASE_KEY + i, 0666);
				msgctl(q_id[i], IPC_RMID, NULL);
			}
			sem_close(mat_lock);

			kill((pid_t) 0, SIGKILL);
			return 0;
		}
		printf("manager: No deadlock detected\n");
	}
	return 0;
}
