#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct thread_params {
	int i, k, n;
};

int** dist;

void printOutput(int n) {
	printf("Printing output...\n");
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			printf("%d	", dist[i][j]);
		}
		printf("\n");
	}
}

void readInput(int* nptr) {
	printf("Reading input...\n");
	int m, n;
	scanf("%d %d", &n, &m);
	*nptr = n;

	dist = (int**) malloc(n * sizeof(int*));
	for(int i = 0; i < n; i++) {
		dist[i] = (int*) malloc(n * sizeof(int));
		for(int j = 0; j < n; j++) {
			if(i != j) dist[i][j] = (int) -1;
			else dist[i][j] = 0;
		}
	}

	int i, j;
	int w;
	while(m--) {
		scanf("%d %d %d", &i, &j, &w);
		dist[i][j] = dist[j][i] = w;
	}
}

void updateDist(int i, int j, int k) {
	//TODO: wait on semaphore and then update dist
	printf("\nupdateDist(%d, %d, %d):\n", i, j, k);
	// printOutput(4);
	printf("dist[i][j] = %d, dist[i][k] = %d, dist[k][j] = %d\n", dist[i][j], dist[i][k], dist[k][j]);
	if(dist[i][k] == -1 || dist[k][j] == -1) {
		printf("updateDist(%d, %d, %d) Not updated\n", i, j, k);
		return;
	}
	if(dist[i][j] == -1 || dist[i][k] + dist[k][j] < dist[i][j]) {
		printf("updateDist(%d, %d, %d) Updated\n", i, j, k);
		dist[i][j] = dist[i][k] + dist[k][j];
		return;
	}
}


void* ij_loop(void* params) {
	struct thread_params* temp = (struct thread_params*) params;
	int i = temp -> i;
	int k = temp -> k;
	int n = temp -> n;

	printf("Thread with i = %d started\n", i);

	for(int j = 0; j < n; j++) {
		printf("Calling updateDist(%d, %d, %d):\n", i, j, k);
		updateDist(i, j, k);
	}
}

int main(int argc, char* argv[]) {
	int n;
	readInput(&n);

	for(int k = 0; k < n; k++) {
		pthread_t thread_id[n];
		for(int i = 0; i < n; i++) {
			struct thread_params temp;
			temp.i = i;
			temp.k = k;
			temp.n = n;
			pthread_create(&thread_id[i], NULL, &ij_loop, &temp);
		}
		for(int i = 0; i < n; i++) {
			pthread_join(thread_id[i], NULL);
		}
	}

	printOutput(n);
	return 0;
}
