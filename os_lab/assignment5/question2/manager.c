#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include "ass5q2.h"

#define MAX_TRAINS 100
#define N 1
#define E 2
#define S 3
#define W 4


void process_sequence(char *sequence, int n){
    for(int i = 0; i < n; i++){
        if(sequence[i] == 'N') sequence[i] = (char)N;
        if(sequence[i] == 'E') sequence[i] = (char)E;
        if(sequence[i] == 'W') sequence[i] = (char)W;
        if(sequence[i] == 'S') sequence[i] = (char)S;
    }
}

int main(int argc, char* argv[]){
    FILE *fp;
    char sequence[MAX_TRAINS];
    int n;                                      //length of the sequence
    int m = NUM_DIRECTION;                                  //no. of semaphores
    int **matrix;
    sem_t* direction_lock[NUM_DIRECTION];
    sem_t* mat_lock;
    int *train_pid;

    int train_count = 0;

    if(argc < 2){
        printf("Usage: manager <p value> <sequence.txt> \n");
        return -1;
    }

    fp = fopen(argv[2], 'r');
    if(fp == NULL)
    {
        perror("Error: ");
        return(-1);
    }

    fscanf(fp, "%s", sequence);
    fclose(fp);

    n = strlen(sequence);

    init_matrix(matrix, n);
    process_sequence(sequence, n);

    sem_unlink("mat_lock");                  //destroy the existing semaphores, if any
    mat_lock = sem_open("mat_lock", O_CREAT, O_RDWR, 1);
    sem_wait(mat_lock);
    FILE* matrix_file = fopen("matrix.txt", "w+");
    writeMatrix(matrix_file, matrix, n);
    fflush(matrix_file);
    readMatrix(matrix_file, matrix, n);
    printf("manager: Matrix file initialized to:\n");
    writeMatrix(stdout, matrix, n);
    fclose(matrix_file);
    sem_post(mat_lock);


    sem_unlink("north_lock");                //destroy the existing semaphores, if any
    sem_unlink("west_lock");
    sem_unlink("east_lock");
    sem_unlink("south_lock");
    direction_lock[N] = sem_open("north_lock", O_CREAT, O_RDWR, 1);        //Creating north_lock semaphore
    if(direction_lock[N]== SEM_FAILED) {
        perror("Could not start semaphore\n");
        return 0;
    }
    direction_lock[E] = sem_open("east_lock", O_CREAT, O_RDWR, 1);
    if(direction_lock[E] == SEM_FAILED) {
        perror("Could not start semaphore\n");
        return 0;
    }
    direction_lock[W] = sem_open("west_lock", O_CREAT, O_RDWR, 1);
    if(direction_lock[W] == SEM_FAILED) {
        perror("Could not start semaphore\n");
        return 0;
    }
    direction_lock[S] = sem_open("south_lock", O_CREAT, O_RDWR, 1);
    if(direction_lock[S] == SEM_FAILED) {
        perror("Could not start semaphore\n");
        return 0;
    }

    train_pid = (int*)malloc(n*sizeof(int));            //Allocate memory

    while(1){

        if(train_count < n)
        {//While all the trains are not created
            if(coinToss(atoi(argv[1])))
            {   //check for deadlocks with probability
                sem_wait(mat_lock);
                matrix_file = fopen("matrix.txt", "r+");
                fflush(matrix_file);
                readMatrix(matrix_file, matrix, n);
                writeMatrix(stdout, matrix, n);
                fclose(matrix_file);
                sem_post(mat_lock);
            }
            else
            {   //Create train with probability 1-p
                train_pid[train_count] = fork();
                train_count++;
                if(train_pid[train_count] == 0) {
                    char* args[4];
                    args[0] = "train";
                    sprintf(args[1], "%d", train_count);
                    sprintf(args[2], "%d", sequence[train_count]);
                    sprintf(args[3], "%d", n);
                    execv(args[0], args);
                    perror("Could not start train: ");
                }
            }

        }
        else
        {  //after all the trains are created, only check for deadlocks
            sem_wait(mat_lock);
            matrix_file = fopen("matrix.txt", "r+");
            fflush(matrix_file);
            readMatrix(matrix_file, matrix, n);
            writeMatrix(stdout, matrix, n);
            fclose(matrix_file);
            sem_post(mat_lock);
        }

    }

    return 0;
}
