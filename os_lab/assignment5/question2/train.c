#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ass5q2.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define N 1
#define E 2
#define S 3
#define W 4

#define RIGHT(i) (i+1)%4

sem_t* direction_lock[4];
sem_t* mat_lock;
sem_t* mutex;

typedef struct train{
    int id;
    int direction;
}train;

void load_semaphores(){

    //load semaphores
    mat_lock = sem_open("mat_lock", O_CREAT, O_RDWR, 1);
    direction_lock[N] = sem_open("north_lock", O_CREAT, O_RDWR, 1);        //Creating north_lock semaphore
    direction_lock[E] = sem_open("east_lock", O_CREAT, O_RDWR, 1);
    direction_lock[W] = sem_open("west_lock", O_CREAT, O_RDWR, 1);
    direction_lock[S] = sem_open("south_lock", O_CREAT, O_RDWR, 1);
    mutex = sem_open("mutex", O_CREAT, O_RDWR, 1);
}

int main(int argc, char* argv[]){

    train t;
    int n = atoi(argv[3]);

    t.id = atoi(argv[1]);
    t.direction = atoi(argv[2]);

    load_semaphores();
    printf("Train id: %d with direction %d started\n", t.id, t.direction);

    //file ptr to the matrix file
    FILE* matrix_file = fopen("matrix.txt", "r+");

    srand(time(NULL));
    sem_wait(mat_lock);
    updateMatrix(matrix_file, t.id, t.direction, REQUESTED, n);
    printf("Train id: %d requests for direction %d lock\n", t.id, t.direction);
    sem_post(mat_lock);

    sem_wait(direction_lock[t.direction]);

    sem_wait(mat_lock);
    updateMatrix(matrix_file, t.id, t.direction, ACQUIRED, n);
    printf("Train id: %d acquired direction %d lock\n", t.id, t.direction);
    sem_post(mat_lock);



    sem_wait(mat_lock);
    updateMatrix(matrix_file, t.id, RIGHT(t.direction), REQUESTED, n);                   //wait on semaphore of its own direction
    printf("Train id %d requests for direction %d(its right) lock\n",t.id, RIGHT(t.direction));
    sem_post(mat_lock);

    sem_wait(direction_lock[RIGHT(t.direction)]);            //wait on semaphore of its right direction

    sem_wait(mat_lock);
    updateMatrix(matrix_file, t.id, RIGHT(t.direction), REQUESTED, n);                   //wait on semaphore of its own direction
    printf("Train id %d acquired direction %d(its right) lock\n", t.id, RIGHT(t.direction));
    sem_post(mat_lock);

    sem_wait(mutex);                                        //checks mutual exclusion at the junction

    sleep(2);                                                //train crossing the junction

    sem_post(mutex);

    sem_wait(mat_lock);
    updateMatrix(matrix_file, t.id, RIGHT(t.direction), NA, n);                   //wait on semaphore of its own direction
    printf("Train id %d releases direction %d(its right) lock\n", t.id, RIGHT(t.direction));
    sem_post(mat_lock);                                                        //release the mutual exlusion lock

    sem_post(direction_lock[t.direction]);                   //release the semaphore of its own direction

    sem_wait(mat_lock);
    updateMatrix(matrix_file, t.id, t.direction, NA, n);                   //wait on semaphore of its own direction
    printf("Train id: %d acquired direction %d lock\n", t.id, RIGHT(t.direction));
    sem_post(mat_lock);

    sem_post(direction_lock[RIGHT(t.direction)]);            //release the semaphore of its right direction

    return 0;
}
