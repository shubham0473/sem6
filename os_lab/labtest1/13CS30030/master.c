#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
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
        edge[i].start = temp1;
        edge[i].end = temp2;
        pipe(edge[i].pipe_u);
        pipe(edge[i].pipe_v);
    }


    int parent_pid = getpid();
    int sum = 0;

    int nodepid[N+1];
    printf("here\n");

    for(int i = 1; i <= N; i++){
        int rand_val = rand() % 100 +1;
        nodepid[i] = fork();
        if(nodepid[i] == 0){
            printf("starting node\n");
            char* args[7];
            char name[50];
            sprintf(name, "file_desc%d", i);
            FILE *fp = fopen(name, "w+");
            if(fp == NULL) exit(0);
            for(int j = 0; j < 6; j++){
                args[j] = (char*)malloc(50*sizeof(char));
            }
            int neighbr_cnt = 0;
            for(int j = 0; j < M; j++){
                if(edge[j].start == i) {
                    fprintf(fp, "%d %d\n", edge[j].pipe_v[0], edge[j].pipe_u[1]);
                    neighbr_cnt++;
                    // asprintf(&args[6 + neighbr_cnt++], "%d %d ", edge[j].pipe_v[0], edge[j].pipe_u[1]);

                }
                else if(edge[j].end == i){

                    fprintf(fp, "%d %d\n", edge[j].pipe_u[0], edge[j].pipe_v[1]);
                    neighbr_cnt++;
                    // asprintf(&args[6 + neighbr_cnt++], "%d %d ", edge[j].pipe_u[0], edge[j].pipe_v[1]);
                }
            }
            args[6] = NULL;
            fflush(fp);

            fclose(fp);

            sprintf(args[1], "%d", i);
            sprintf(args[2], "%d", rand_val);
			sprintf(args[3], "%d", parent_pid);
			sprintf(args[4], "%d", N);
			sprintf(args[5], "%d", neighbr_cnt);
            sprintf(args[0], "node.out");
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
        waitpid(nodepid[i], &status, 0);
        int result = WEXITSTATUS(status);
        if(result == mean) printf("Node %d calculate = %d, actual = %d\n", i, result, mean);
    }
    printf("\n");

    return 0;
}
