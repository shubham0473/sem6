#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
#define MAX_NODES 100



typedef struct node{
    int val;
    int id;
}node;

typedef struct Nodevec {
    int size;
    node v[MAX_NODES];
}Nodevec;


int main(int argc, char *argv[]){

    int nodeid = atoi(argv[1]);
    int val = atoi(argv[2]);
    int parent_pid = atoi(argv[3]);
    int N = atoi(argv[4]);
    int neighbr_cnt = atoi(argv[5]);

    char * line = NULL;
    size_t len = 0;
    ssize_t r;

    int fd[neighbr_cnt][2];

    char name[50];
    sprintf(name, "file_desc%d", nodeid);
    FILE *fp = fopen(name, "r");
    if(fp == NULL) exit(0);


    for(int i = 0; i < neighbr_cnt; i++){
        if((r = getline(&line, &len, fp)) == -1) break;
        int temp = sscanf(line, "%d %d", &fd[i][0], &fd[i][1]);
        fcntl(fd[i][0], F_SETFL, O_NONBLOCK);
    }
    printf("\n");

    fclose(fp);
    if (line)
        free(line);

    int neigh_val[MAX_NODES];
    for(int i = 0; i < MAX_NODES; i++) neigh_val[i] = -1;
    int total_nodes = 0;

    Nodevec nodevec;
    nodevec.v[0].id = nodeid;
    nodevec.v[0].val = val;
    nodevec.size = 1;

    for(int j = 0; j < N; j++) {
        usleep(10000);
        for(int i = 0; i < neighbr_cnt; i++){
            write(fd[i][1], &nodevec, sizeof(nodevec));
        }

        for(int i = 0; i < nodevec.size; i++) {
            if(neigh_val[nodevec.v[i].id] != -1) continue;
            neigh_val[nodevec.v[i].id] = nodevec.v[i].val;
            printf("Node %d copied (%d, %d)\n", nodeid, nodevec.v[i].id, nodevec.v[i].val);
            total_nodes++;
        }
        for(int i = 0; i < neighbr_cnt; i++){
            Nodevec recvd_vec;
            if(read(fd[i][0], &recvd_vec, sizeof(recvd_vec)) == -1) continue;
            for(int k = 0; k < recvd_vec.size; k++){
                // recvd_vec.size--;
                printf("Node %d received (%d, %d)\n", nodeid, recvd_vec.v[k].id, recvd_vec.v[k].val);
                if(neigh_val[recvd_vec.v[k].id] == -1) {
                    nodevec.v[nodevec.size++] = recvd_vec.v[k];
                }
            }
        }

    }

    int sum = 0;

    for(int i = 1; i <= N; i++){
        printf("Node %d (%d, %d)\n", nodeid, i, neigh_val[i]);
        if(neigh_val[i] != -1){
            sum += neigh_val[i];
        }
    }

    int mean = sum / N;

    printf("Node %d : total nodes = %d, mean =%d\n", nodeid, total_nodes, mean);

    return(mean);
}
