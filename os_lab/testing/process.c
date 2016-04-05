#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>

struct shmstruct {
	int value;
	char message[50];
};

int main() {
	key_t shmkey = ftok("manager.c", 1);
	key_t semkey = ftok("manager.c", 2);

	int shmid = shmget(shmkey, sizeof(struct shmstruct), 0666);
	int semid = semget(semkey, 2, 0666);

	printf("process: shmid = %d, semid = %d\n", shmid ,semid);

	for(int i = 0; i < 10; i++) {
		struct sembuf sop;
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		printf("process: waiting on semaphore\n");
		semop(semid, &sop, 1);
		printf("process: locked on semaphore\n");


		struct shmstruct* shm = (struct shmstruct*) shmat(shmid, NULL, 0);
		if(shm == (void*) -1) {
			printf("process: shmat failed\n");
		}
		printf("process: shm attached\n");
		printf("process: shm = {%d}\n", shm -> value);
		printf("process: shm.message = {%s}\n", shm -> message);

		// shm -> message = strdup("process.c");
		// asprintf(&(shm->message), "process.c");
		strcpy(shm -> message, "process.c");
		shm -> value = i;
		sleep(1);
		shmdt(shm);

		sop.sem_num = 0;
		sop.sem_op = 1;
		sop.sem_flg = 0;
		printf("process: releasing semaphore\n");
		semop(semid, &sop, 1);
		printf("process: released semaphore\n");
	}

	return 0;
}
