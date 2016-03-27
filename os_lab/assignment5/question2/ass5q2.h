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

// //Allocate memory and initialize matrix
// void init_matrix(int **matrix, int n){
// 	int i, j;
//
// 	matrix = (int**)malloc(NUM_DIRECTION*sizeof(int*));
// 	for(i = 0; i < NUM_DIRECTION; i++){
// 		matrix[i] = (int*)malloc(n*sizeof(int));
// 	}
//
// 	for(i = 0; i < NUM_DIRECTION; i++){
// 		for(j = 0; j < n; j++){
// 			matrix[j][i] = NA;
// 		}
// 	}
// }

//Allocate memory and initialize matrix
void print_matrix(int matrix[][NUM_DIRECTION], int n){
	int i, j;

	for(i = 0; i < n; i++){
		for(j = 0; j < NUM_DIRECTION; j++){
			printf("%d  ", matrix[i][j]);
		}
		printf("\n");
	}
}

void readMatrix(FILE* matrix_file, int matrix[][NUM_DIRECTION], int n) {
	rewind(matrix_file);
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < NUM_DIRECTION; j++) {
			fscanf(matrix_file, "%d", &matrix[i][j]);
			// printf("stored value %d\n", matrix[i][j]);
		}
	}
}

void writeMatrix(FILE* matrix_file, int matrix[][NUM_DIRECTION], int n) {
	rewind(matrix_file);
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < NUM_DIRECTION; j++) {
			// printf("writing %d\n", matrix[i][j]);
			fprintf(matrix_file, "%d	", matrix[i][j]);
		}
		fprintf(matrix_file, "\n");
	}
}

void updateMatrix(FILE* matrix_file, int train_id, int direction, int state, int n)	{
	fflush(matrix_file);
	int matrix[n][NUM_DIRECTION];
	readMatrix(matrix_file, matrix, n);
	matrix[train_id][direction] = state;
	writeMatrix(matrix_file, matrix, n);
	fflush(matrix_file);
	// printf("Matrix updated to: \n");
	// print_matrix(matrix, n);
}

//NOTE : This check cycle function is Hardcoded for our use, it will detect a cycle of length 8
int checkCycle(int matrix[][NUM_DIRECTION], int *cycle, int n) {
	int flag = 0;
	int tot = 0;
	for(int i = 0; i < n; i++){
		if(matrix[i][0] == 2 && matrix[i][1] == 1){
			cycle[tot++] = i;
			cycle[tot++] = 1;
			flag++;
		}
		if(matrix[i][1] == 2 && matrix[i][2] == 1){
			cycle[tot++] = i;
			cycle[tot++] = 2;
			flag++;
		}
		if(matrix[i][2] == 2 && matrix[i][3] == 1){
			cycle[tot++] = i;
			cycle[tot++] = 3;
			flag++;
		}
		if(matrix[i][3] == 2 && matrix[i][0] == 1){
			cycle[tot++] = i;
			cycle[tot++] = 0;
			flag++;
		}

		if(flag == 4) return 1;
	}
	return 0;

}

void printCycle(int cycle[]) {
	for(int i = 0; i < 8; i += 2) {
		printf("Train %d waiting for direction %d taken by train %d\n", cycle[i], cycle[(i + 1) % 8], cycle[(i + 2) % 8]);
	}
	// printf("train %d waiting for -> directiontrain %d waiting for -> train %d waiting for ->train %d waiting for -> train %d waiting for -> train %d waiting for -> train %d waiting for -> train %d waiting for -> train %d waiting for ->\n\n", cycle[0], cycle[1], cycle[2], cycle[3], cycle[4], cycle[5], cycle[6], cycle[7], cycle[0]);
	return;
}
