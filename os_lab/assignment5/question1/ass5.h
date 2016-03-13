#include <stdio.h>
#include <stdlib.h>

#define NUM_Q 2 // Number of queues
#define NUM_PC 5 // Number of producers/consumers

#define Q_BASE_KEY 131
#define Q1_KEY 131
#define Q2_KEY 132

#define STATE_NONE 0
#define STATE_WAIT 1
#define STATE_LOCK 2

#define Q_SIZE 10

typedef struct MESSAGE {
	long mtype;
	char mtext[10];
} message;


// NOTE: Hardcoded for NUM_Q queues and 10 processes, add params if needed
void readMatrix(FILE* matrix_file, int producer_states[][NUM_Q], int consumer_states[][NUM_Q]) {
	rewind(matrix_file);
	for(int i = 0; i < NUM_Q; i++) {
		for(int j = 0; j < NUM_PC; j++) {
			fscanf(matrix_file, "%d", &producer_states[j][i]);
		}
		for(int j = 0; j < NUM_PC; j++) {
			fscanf(matrix_file, "%d", &consumer_states[j][i]);
		}
	}
}

// NOTE: Hardcoded for NUM_Q queues and 10 processes, add params if needed
void writeMatrix(FILE* matrix_file, int producer_states[][NUM_Q], int consumer_states[][NUM_Q]) {
	rewind(matrix_file);
	for(int i = 0; i < NUM_Q; i++) {
		for(int j = 0; j < NUM_PC; j++) {
			fprintf(matrix_file, "%d	", producer_states[j][i]);
		}
		for(int j = 0; j < NUM_PC; j++) {
			fprintf(matrix_file, "%d	", consumer_states[j][i]);
		}
		fprintf(matrix_file, "\n");
	}
}

void updateMatrix(FILE* matrix_file, int process_id, int queue, int state)	{
	fflush(matrix_file);
	// fflush(stdout);
	// printf("\n\n\nupdateMatrix[%d, %d, %d] called:\n", process_id, queue, state);

	int producer_states[NUM_PC][NUM_Q], consumer_states[NUM_PC][NUM_Q];
	readMatrix(matrix_file, producer_states, consumer_states);
	// printf("Read matrix:\n");
	// writeMatrix(stdout, producer_states, consumer_states);

	if(process_id < NUM_PC) producer_states[process_id][queue] = state;
	else consumer_states[process_id - NUM_PC][queue] = state;
	writeMatrix(matrix_file, producer_states, consumer_states);
	// printf("[%d, %d] updated to %d:\n", process_id, queue, state);
	// writeMatrix(stdout, producer_states, consumer_states);
	fflush(matrix_file);
	// fflush(stdout);
}

// Used for debugging
// void printMatrix(FILE* matrix_file)	{
// 	int producer_states[NUM_PC][NUM_Q], consumer_states[NUM_PC][NUM_Q];
// 	readMatrix(matrix_file, producer_states, consumer_states);
// 	writeMatrix(stdout, producer_states, consumer_states);
// }

// Returns a 1 with probability p, 0 with prob 1-p
// NOTE: srand should be called by caller
int coinToss(double p) {
	return (rand() < p * RAND_MAX) ? 1 : 0;
}

int checkCycle(int consumer_states[][NUM_Q], int cycle[]) {
	int flagA = 0, flagB = 0;
	int count = 0;
	for(int i = 0; i < NUM_PC; i++) {
		if(consumer_states[i][0] == 1 && consumer_states[i][1] == 2) {
			cycle[count++] = i;
			cycle[count++] = 0;
			flagA = 1;
		}
		if(consumer_states[i][0] == 2 && consumer_states[i][1] == 1) {
			cycle[count++] = i;
			cycle[count++] = 1;
			flagB = 1;
		}
	}
	return flagA && flagB;
}

void printCycle(int cycle[]) {
	printf("C%d -> Q%d -> C%d -> Q%d -> C%d\n\n", cycle[0], cycle[1], cycle[2], cycle[3], cycle[0]);
	return;
}
