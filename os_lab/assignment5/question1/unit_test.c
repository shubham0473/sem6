#include "ass5.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
	int producer_states[5][2], consumer_states[5][2];
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < 5; j++) {
			producer_states[j][i] = i * 20 + j;
		}
		for(int j = 0; j < 5; j++) {
			consumer_states[j][i] = i * 20 + j + 10;
		}
	}

	FILE* matrix_file = fopen("matrix.txt", "w+");

	printf("Data written to file:\n");
	writeMatrix(matrix_file, producer_states, consumer_states);
	writeMatrix(stdout, producer_states, consumer_states);

	// clear states for testing read
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < 5; j++) {
			producer_states[j][i] = 0;
		}
		for(int j = 0; j < 5; j++) {
			consumer_states[j][i] = 0;
		}
	}

	printf("Data read from file:\n");
	readMatrix(matrix_file, producer_states, consumer_states);
	writeMatrix(stdout, producer_states, consumer_states);

	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < 5; j++) {
			assert(producer_states[j][i] == i * 20 + j);
		}
		for(int j = 0; j < 5; j++) {
			assert(consumer_states[j][i] == i * 20 + j + 10);
		}
	}

	updateMatrix(matrix_file, 2, 0, 100);
	updateMatrix(matrix_file, 8, 1, 100);
	printf("Data read from file:\n");
	readMatrix(matrix_file, producer_states, consumer_states);
	writeMatrix(stdout, producer_states, consumer_states);
	assert(producer_states[2][0] == 100);
	assert(consumer_states[3][1] == 100);

	printf("Assertions successful!\n");
	fclose(matrix_file);
	return 0;
}
