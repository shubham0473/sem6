#include <sys/types.h>
#include "ass5q2.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define N 0
#define E 1
#define S 2
#define W 3

#define RIGHT(i) (i+1)%4



typedef struct train{
    int id;
    int direction;
}train;

// void load_semaphores(){
//
//
// }

int main(int argc, char* argv[]){

    train t;
    sem_t* direction_lock[4];
    sem_t* matrix_lock;
    sem_t* mutex;
    int n = atoi(argv[3]);

    t.id = atoi(argv[1]);
    t.direction = atoi(argv[2]);

    // load_semaphores();
    //load semaphores
    matrix_lock = sem_open("matrix_lock", O_CREAT, O_RDWR, 1);
    if(matrix_lock == SEM_FAILED) {
        perror("Could not start semaphore\n");
        exit(0);
    }
    direction_lock[N] = sem_open("north_lock", O_CREAT, O_RDWR, 1);        //Creating north_lock semaphore
    if(direction_lock[N]  == SEM_FAILED) {
        perror("Could not start semaphore\n");
        exit(0);
    }
    direction_lock[E] = sem_open("east_lock", O_CREAT, O_RDWR, 1);
    if(direction_lock[E]  == SEM_FAILED) {
        perror("Could not start semaphore\n");
        exit(0);
    }
    direction_lock[W] = sem_open("west_lock", O_CREAT, O_RDWR, 1);
    if(direction_lock[W]  == SEM_FAILED) {
        perror("Could not start semaphore\n");
        exit(0);
    }
    direction_lock[S] = sem_open("south_lock", O_CREAT, O_RDWR, 1);
    if(direction_lock[S]  == SEM_FAILED) {
        perror("Could not start semaphore\n");
        exit(0);
    }
    mutex = sem_open("mutex", O_CREAT, O_RDWR, 1);
    if(mutex == SEM_FAILED) {
        perror("Could not start semaphore\n");
        exit(0);
    }
    // printf("Successfully loaded semaphores\n");
    printf("Train %d: with direction %d started\n", t.id, t.direction);

    //file ptr to the matrix file
    FILE* matrix_file = fopen("matrix.txt", "r+");

    srand(time(NULL));

	//// GET LOCK ON OWN DIR
    sem_wait(matrix_lock);
    updateMatrix(matrix_file, t.id, t.direction, 1, n);
    printf("Train %d: requests for direction %d lock\n", t.id, t.direction);
    sem_post(matrix_lock);
    sem_wait(direction_lock[t.direction]);
    sem_wait(matrix_lock);
    updateMatrix(matrix_file, t.id, t.direction, 2, n);
    printf("Train %d: acquired direction %d lock\n", t.id, t.direction);
    sem_post(matrix_lock);

	//// GET LOCK ON RIGHT DIR
    sem_wait(matrix_lock);
    updateMatrix(matrix_file, t.id, RIGHT(t.direction), 1, n);
    printf("Train %d: requests for direction %d(its right) lock\n",t.id, RIGHT(t.direction));
    sem_post(matrix_lock);
    sem_wait(direction_lock[RIGHT(t.direction)]);
    sem_wait(matrix_lock);
    updateMatrix(matrix_file, t.id, RIGHT(t.direction), 2, n);
    printf("Train %d: acquired direction %d(its right) lock\n", t.id, RIGHT(t.direction));
    sem_post(matrix_lock);

	//// CROSS JUNCTION
	printf("Train %d: requests for junction lock\n", t.id);
    sem_wait(mutex);
	printf("Train %d: acquires junction lock, crossing now\n", t.id);
    sleep(2);
    sem_post(mutex);
	printf("Train %d: requests for junction lock\n", t.id);

	//// RELEASE OWN DIR LOCK
    sem_wait(matrix_lock);
    updateMatrix(matrix_file, t.id, t.direction, 0, n);
    printf("Train %d: releases direction %d lock\n", t.id, t.direction);
    sem_post(matrix_lock);
    sem_post(direction_lock[t.direction]);

	//// RELEASE RIGHT LOCK
    sem_wait(matrix_lock);
    updateMatrix(matrix_file, t.id, RIGHT(t.direction), 0, n);
    printf("Train %d: releases direction %d(its right) lock\n", t.id, RIGHT(t.direction));
    sem_post(matrix_lock);
    sem_post(direction_lock[RIGHT(t.direction)]);

    fclose(matrix_file);
    return 0;
}
