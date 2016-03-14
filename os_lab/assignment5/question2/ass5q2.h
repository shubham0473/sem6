#include <stdio.h>
#include <stdlib.h>

#define NA 0
#define REQUESTED 1
#define ACQUIRED 2
#define NUM_DIRECTION 4


// Returns a 1 with probability p, 0 with prob 1-p
// NOTE: srand should be called by caller
int coinToss(double p) {
	return (rand() < p * RAND_MAX) ? 1 : 0;
}

//Allocate memory and initialize matrix
void init_matrix(int **matrix, int n){
	int i, j;

	matrix = (int**)malloc(n*sizeof(int*));
	for(i = 0; i < n; i++){
		matrix[i] = (int*)malloc(NUM_DIRECTION*sizeof(int));
	}

	for(i = 0; i < n; i++){
		for(j = 0; j < NUM_DIRECTION; j++){
			matrix[i][j] = 0;
		}
	}
}

void readMatrix(FILE* matrix_file, int **matrix, int n) {
	rewind(matrix_file);
	for(int i = 0; i < NUM_DIRECTION; i++) {
		for(int j = 0; j < n; j++) {
			fscanf(matrix_file, "%d", &matrix[j][i]);
		}
	}
}

void writeMatrix(FILE* matrix_file, int **matrix, int n) {
	rewind(matrix_file);
	for(int i = 0; i < NUM_DIRECTION; i++) {
		for(int j = 0; j < n; j++) {
			fprintf(matrix_file, "%d	", matrix[j][i]);
		}
		fprintf(matrix_file, "\n");
	}
}

void updateMatrix(FILE* matrix_file, int train_id, int direction, int state, int n)	{
	fflush(matrix_file);
	int matrix[NUM_DIRECTION][n];
	readMatrix(matrix_file, matrix, n);
	matrix[direction][train_id] = state;
	writeMatrix(matrix_file, matrix, n);
	fflush(matrix_file);
}

//NOTE : This check cycle function is Hardcoded for our use, it will detect a cycle of length 8
int checkCycle(int **matrix, int *cycle, int n) {
	int flag = 0, j = 0;

	for(int i = 0; i < n; i++){
		if(matrix[(i+1)%4][i] == ACQUIRED && matrix[(i+1)%4][i] == REQUESTED){
			cycle[j++] = matrix[i][i];
			cycle[j++] = matrix[i+1][i+1];
			if(j == 7) flag = 1;
		}
	}
	return flag;
}

void printCycle(int cycle[]) {
	printf("train %d waiting for -> train %d waiting for -> train %d waiting for ->train %d waiting for -> train %d waiting for -> train %d waiting for -> train %d waiting for -> train %d waiting for -> train %d waiting for ->\n\n", cycle[0], cycle[1], cycle[2], cycle[3], cycle[4], cycle[5], cycle[6], cycle[7], cycle[0]);
	return;
}
