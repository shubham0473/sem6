/**********************************************
* evaluate.c
* Runs manager.c for different values of p
* and calculates the average number of insertions
* consumtions over 10 runs

* Recommended: lower sleep time in manager before
* increasing number of runs
************************************************/

#include <stdio.h> // fopen, sprintf
#include <unistd.h> // fork, execv
#include <sys/types.h> // wait
#include <sys/wait.h> // wait
#include <assert.h>
#include <stdlib.h> // malloc

int main() {
	FILE* results_file = fopen("results.txt", "w+");
	for(double p = 0.1; p < 0.91; p += 0.1) {
		printf("Starting runs for p = %lf\n", p);
		int inserts_sum = 0, consumes_sum = 0;
		for(int i = 0; i < 100; i++) {
			printf("Run no. %d\n", i);
			int cid = fork();
			if(cid == 0) {
				char* args[4];
				args[0] = "manager";
				args[1] = (char*) malloc(sizeof(char) * 10);
				sprintf(args[1], "%lf", p);
				args[2] = (char*) malloc(sizeof(char) * 10);
				sprintf(args[2], "%d", 0); // deadlock solution
				args[3] = NULL;
				execv(args[0], args);
			}
			waitpid(cid, NULL, 0);
			FILE* tempfile = fopen("temp.txt", "r");
			double temp;
			int inserts, consumes;
			fscanf(tempfile, "%lf	%d	%d", &temp, &inserts, &consumes);
			printf("p:%lf	temp:%lf	inserts:%d	consumes:%d\n", p, temp, inserts, consumes);
			assert(temp - p < 0.001);
			inserts_sum += inserts;
			consumes_sum += consumes;
			fclose(tempfile);
		}

		int avg_inserts = inserts_sum / 100;
		int avg_consumes = consumes_sum / 100;

		fprintf(results_file, "p:%lf	avg_inserts:%d	avg_consumes:%d\n", p, avg_inserts, avg_consumes);
	}
	fclose(results_file);
	return 0;
}
