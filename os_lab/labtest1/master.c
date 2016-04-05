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


typedef struct graphedge{
    int start;
    int end;
    int pipe_u[2];
    int pipe_v[2];
} graphedge;


int main(int argc, char* argv[]){
    int N, M;

    srand(time(NULL));
    printf("Please enter  N and M respectively\n");
    scanf("%d %d", &N, &M);

    graphedge edge[M];

    for(int i = 0; i < M; i++){
        int temp1, temp2;
        scanf("%d %d", &temp1, &temp2);
        edge[i].start = start;
        edge[i].end = end;
        pipe(edge[i].pipe_u);
        pipe(edge[i].pipe_v)
    }


    int parent_pid = getpid();
    int sum = 0;

    int nodepid[N];

    for(int i = 0; i < N; i++){
        int rand_val = rand()%100 +1;
        nodepid[i] = fork();
        if(nodepid[i] == 0){
            char* args[MAX_NODES + 6];
            int neighbr_cnt = 0;
            for(int j = 0; j < M; j++){
                if(edge[j].end == i) {
                    asprintf(&args[6 + neighbr_cnt++], "%d %d ", edge[j].pipe_v[0], edge[j].pipe_u[1]);

                }
                else if(edge[i].end == i){
                    asprintf(&args[6 + neighbr_cnt++], "%d %d ", edge[j].pipe_u[0], edge[j].pipe_v[1]);
                }
            }
            args[6 + neighbr_cnt] = NULL;

            asprintf(&args[1], "%d", i);
            asprintf(&args[2], "%d", random_value);
			asprintf(&args[3], "%d", parent_pid);
			asprintf(&args[4], "%d", nbr_count);
			asprintf(&args[5], "%d", N);
            asprintf(&args[0], "node.out");
            execv(args[0], args);
            printf("EXEC FAILED\n");

        }
        else {
            printf("Node %d with rand val %d started\n", i, rand_val);
            fflush(stdout);
            sum += rand_val;
        }

        if(i == N) break;
    }

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
