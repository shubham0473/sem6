/******************************************
Name: Divyansh Gupta
Roll: 13CS30011
M/c: 39
Code: 13CS30011_master.c

NOTE: Bonus part solved:
Number of iterations required is equal to
diameter of graph
******************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#define MAX_NODES 100


typedef struct EDGE {
	int start;
	int end;
	int fwd[2];
	int bwd[2];
} edgetype;

volatile int num_ready; //number of child processes that are ready

int main(int argc, char* argv[]) {
	int N, M;
	srand(time(NULL));
	scanf("%d %d", &N, &M);

	int xterm = 1;
	int xterm_offset = 0;

	if(argc < 2) {
		printf("\nUsage: $ ./master.out [1 to use with xterm, 0 without] < master_input_file.txt\n");
		printf("\nNOTE: xterms make it easier to see the logic of every node/process. However, they will not be able to return the calculated mean to the master\n");
		exit(0);
	}
	if(!strcmp("0", argv[1])) xterm = 0;
	// else xterm = 0;

	edgetype edge[M]; // M edges

	int node[N + 1]; // N nodes

	for(int i = 0; i < M; i++) {
		int start, end;
		scanf("%d %d", &start, &end);
		edge[i].start = start;
		edge[i].end = end;
		pipe(edge[i].fwd);
		// printf("%d writes on %d, %d receives on %d\n", start, edge[i].fwd[1], end, edge[i].fwd[0]);
		pipe(edge[i].bwd);
		// printf("%d writes on %d, %d receives on %d\n", end, edge[i].bwd[1], start, edge[i].bwd[0]);
	}

	int root = getpid();
	int sum = 0;
	if(xterm) xterm_offset = 3;
	printf("Initial values of nodes:");

	for(int i = 1; i <= N; i++)
	{
		// printf("Startof loop\n");
		int random_value = rand() % 100 + 1; //generete random value for node
		node[i] = fork();
		if(node[i] == 0) { //child process started
			// int offset = 6; // without xterm
			int pipe_offset = 6 + xterm_offset; // inside xterm
			char* args[MAX_NODES + pipe_offset];
			int nbr_count = 0;

			for(int j = 0; j < M; j++) { //copy all file descriptors to node.c args
				if(edge[j].start == i) {
					// char fd[10];
					asprintf(&args[pipe_offset + nbr_count++], "%d %d ", edge[j].bwd[0], edge[j].fwd[1]);
					// strcat(nbr_pipes, fd);
					// nbr_count++;
				}
				else if(edge[j].end == i) {
					// char fd[10];
					asprintf(&args[pipe_offset + nbr_count++], "%d %d ", edge[j].fwd[0], edge[j].bwd[1]);
					// strcat(nbr_pipes, fd);
					// nbr_count++;
				}
			}
			args[pipe_offset + nbr_count] = NULL;
			// printf("Master: nodeno %d, nbr_pipes = >>%s<<\n", i, nbr_pipes);

			asprintf(&args[xterm_offset + 1], "%d", random_value);
			asprintf(&args[xterm_offset + 2], "%d", root);
			asprintf(&args[xterm_offset + 3], "%d", nbr_count);
			asprintf(&args[xterm_offset + 4], "%d", N);
			asprintf(&args[xterm_offset + 5], "%d", i);
			if(xterm) {
				args[0] = strdup("xterm");
				args[1] = strdup("-hold");
				args[2] = strdup("-e");
				asprintf(&args[xterm_offset + 0], "%s/./node.out", getcwd(NULL, 0));
				execv("/usr/bin/xterm", args);
			}
			else {
				asprintf(&args[xterm_offset + 0], "node.out");
				execv(args[0], args);
			}
			printf("Unable to exec\n");
		}
		else {
			printf("(Node %d, %d);", i, random_value);
			fflush(stdout);
			sum += random_value;
		}
		// printf("Endof loop\n");
		if(i == N) break;
	}
	// printf("\n");
	int mean = sum / N;
	printf("\nVerifying mean = %d :", mean);
	fflush(stdout);


	for(int i = 1; i <= N; i++) {
		int status;
		waitpid(node[i], &status, 0);
		int result = WEXITSTATUS(status);
		printf("(Node %d, mean = %d);", i, result);
		assert(result == mean);
	}
	printf("\n");

	return 0;

}
