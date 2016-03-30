/******************************************
Name: Divyansh Gupta
Roll: 13CS30011
M/c: 39
Code: 13CS30011_node.c
******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#define MAX_NODES 100

int nodeno;

// int ready;

// void resume_handler(int SIG) {
// 	printf("Nodeno %d: received SIGUSR1\n", nodeno);
// 	ready = 1;
// }

struct nodedata {
	int nodeno;
	int value;
};

struct vec {
	struct nodedata node[MAX_NODES]; // sum of the nodes communicated in this msg
	int count; // no of nodes communicated
};

int main(int argc, char* argv[]) {

	int nbr_count = atoi(argv[3]);
	// char* nbr_pipes = argv[2];
	int value = atoi(argv[1]);
	int root = atoi(argv[2]);
	int N = atoi(argv[4]);
	nodeno = atoi(argv[5]);

	printf(">>>> nodeno = %d started <<<<\n", nodeno);
	// printf("nbr_count = %d\n", nbr_count);
	// printf("nbr_pipes = %s\n", nbr_pipes);
	// printf("value = %d\n", value);
	// printf("root = %d\n", root);
	// printf("N = %d\n", N);

	int fd[nbr_count][2];
	int offset = 6;

	for(int i = 0; i < nbr_count; i++) {
		int scanned = sscanf(argv[offset + i], "%d %d", &fd[i][0], &fd[i][1]);
		fcntl(fd[i][0], F_SETFL, O_NONBLOCK);
		// printf("Nodeno %d sends on %d, receives on %d\n", nodeno, fd[i][1], fd[i][0]);
	}
	printf("\n");

	// signal(SIGUSR1, &resume_handler);

	int total[MAX_NODES];
	for(int i = 0; i < MAX_NODES; i++) total[i] = -1;
	int total_nodes = 0;

	struct vec current;
	current.node[0].nodeno = nodeno;
	current.node[0].value = value;
	current.count = 1;

	do {
		sleep(1);
		for(int i = 0; i < nbr_count; i++) {
			write(fd[i][1], &current, sizeof(current));
			// printf("Nodeno %d: sent data of %d nodes on %d\n", nodeno, current.count, fd[i][1]);
		}

		while(current.count > 0) {
			// current.count--;
			current.count--;
			if(total[current.node[current.count].nodeno] != -1) continue;

			total[current.node[current.count].nodeno] = current.node[current.count].value;
			printf("Nodeno %d: copied {%d, %d} to total\n", nodeno, current.node[current.count].nodeno, current.node[current.count].value);
			total_nodes++;
		}

		// printf("Nodeno %d: total has %d entries\n", nodeno, total_nodes);
		// printf("Nodeno %d: current has %d entries before recv\n", nodeno, current.count);

		// if(current.count == 0) continue;

		for(int i = 0; i < nbr_count; i++) {
			struct vec recv;
			if(read(fd[i][0], &recv, sizeof(recv)) == -1) continue;
			// printf("Nodeno %d: received %d nodes on %d\n", nodeno, recv.count, fd[i][0]);
			while(recv.count > 0) {
				recv.count--;
				printf("Nodeno %d: received {%d, %d}\n", nodeno, recv.node[recv.count].nodeno, recv.node[recv.count].value);
				if(total[recv.node[recv.count].nodeno] == -1) {
					current.node[current.count++] = recv.node[recv.count];
					// printf("Nodeno %d: copied {%d, %d} to current\n", nodeno, recv.node[recv.count].nodeno, recv.node[recv.count].value);
				}
			}
		}
		// printf("Nodeno %d: current has %d entries after recv\n", nodeno, current.count);

	} while(total_nodes < N);

	int sum;
	for(int i = 1; i <= N; i++) {
		printf("Nodeno %d: total{%d, %d}\n", nodeno, i, total[i]);
		if(total[i] != -1) {

			sum += total[i];
		}
	}
	int mean = sum / N;
	printf("Nodeno %d: FINAL total has %d entries, mean = %d\n", nodeno, total_nodes, mean);

	return(mean);
}
