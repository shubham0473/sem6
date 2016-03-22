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
#define N 0
#define E 1
#define S 2
#define W 3


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
    sem_t* direction_lock[NUM_DIRECTION];
    sem_t* matrix_lock;
    int *train_pid;
    double p;

    int train_count = 0;

    if(argc < 2){
        printf("Usage: manager <p value> <sequence.txt> \n");
        return -1;
    }

    sscanf(argv[1], "%lf", &p);

    fp = fopen(argv[2], "r");
    if(fp == NULL)
    {
        perror("Error: ");
        return(-1);
    }

    fscanf(fp, "%s", sequence);
    fclose(fp);

    n = strlen(sequence);

    int matrix[n][NUM_DIRECTION];

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < NUM_DIRECTION; j++) {
            matrix[i][j] = 0;
        }

    }

    // for(int i = 0; i < NUM_DIRECTION; i++) {
    //     for(int j = 0; j < n; j++) {
    //         printf("%d\t", matrix[j][i]);
    //     }
    //     printf("\n");
    //
    // }

    // init_matrix(matrix, n);
    process_sequence(sequence, n);

    // for(int i = 0; i < n; i++){
    //     printf("%d\n", sequence[i]);
    // }

    sem_unlink("matrix_lock");                  //destroy the existing semaphores, if any
    matrix_lock = sem_open("matrix_lock", O_CREAT, O_RDWR, 1);
    sem_wait(matrix_lock);
    FILE* matrix_file = fopen("matrix.txt", "w+");
    writeMatrix(matrix_file, matrix, n);
    fflush(matrix_file);
    readMatrix(matrix_file, matrix, n);
    printf("manager: Matrix file initialized to:\n");
    // writeMatrix(stdout, matrix, n);
    print_matrix(matrix, n);
    sem_post(matrix_lock);


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
    fflush(matrix_file);
    fclose(matrix_file);
    printf("semaphores done\n");
    while(1){

        if(train_count < n)
        {//While all the trains are not created
            printf("All the trains are not yet created\n");
            if(coinToss(p))
            {   //check for deadlocks with probability
                printf("checking deadlocks\n");
                int cycle[10];
                matrix_file = fopen("matrix.txt", "r+");
                sem_wait(matrix_lock);
                // fflush(matrix_file);
                readMatrix(matrix_file, matrix, n);
                // writeMatrix(stdout, matrix, n);
                print_matrix(matrix, n);
                sem_post(matrix_lock);
                if(checkCycle(matrix, cycle, n) == 1){
                    printf("cycle detected\n");
                    printCycle(cycle);
                    exit(0);
                }
                fclose(matrix_file);
            }
            else
            {   //Create train with probability 1-p
                printf("creating train\n");
                train_pid[train_count] = fork();
                printf("%d\n", train_pid[train_count]);
                if(train_pid[train_count] == 0) {
                    printf("hi child\n");
                    char args[7][100];
                    sprintf(args[0], "%s", "xterm");
                    sprintf(args[1], "%s", "-hold");
                    sprintf(args[2], "%s", "-e");
                    sprintf(args[3], "%s/./train", getcwd(NULL, 0));
                    sprintf(args[4], "%d", train_count);
                    sprintf(args[5], "%d", sequence[train_count]);
                    sprintf(args[6], "%d", n);
                    execl("/usr/bin/xterm", args[0], args[1], args[2], args[3], args[4], args[5],args[6], NULL);
                    perror("Could not start train: ");

                }
                else
                {
                    train_count++;              //increment count in parent
                }
            }

        }
        else
        {  //after all the trains are created, only check for deadlocks
            int cycle[10];
            printf("All trains created, only checking for deadlocks\n");
            sem_wait(matrix_lock);
            matrix_file = fopen("matrix.txt", "r+");
            fflush(matrix_file);
            readMatrix(matrix_file, matrix, n);
            print_matrix(matrix, n);
            sem_post(matrix_lock);

            if(checkCycle(matrix, cycle, n) == 1){
                printf("cycle detected\n");
                printCycle(cycle);
                exit(0);
            }
            sleep(1);
            fclose(matrix_file);
        }
        // sleep(1);

    }
    // fclose(matrix_file);
    return 0;
}
