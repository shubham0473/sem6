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
	semctl(semid, 0, IPC_RMID, NULL);
	shmctl(shmid, IPC_RMID, NULL);

	shmid = shmget(shmkey, sizeof(struct shmstruct), IPC_CREAT | 0666);
	semid = semget(semkey, 2, IPC_CREAT | 0666);
	int values[2] = {1, 1};
	semctl(semid, 0, SETALL, values);

	printf("manager: shmid = %d, semid = %d\n", shmid ,semid);

	struct shmstruct* shm = (struct shmstruct*) shmat(shmid, NULL, 0);
	// shm -> message = strdup("manager.c init");
	// asprintf(&(shm->message), "manager.c init");
	strcpy(shm -> message, "manager.c init");
	shm -> value = -1;
	shmdt(shm);


	if(fork() == 0) {
		execl("process", "process", NULL);
		printf("Unable to start process\n");
	}

	for(int i = 0; i < 10; i++) {
		struct sembuf sop;
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		printf("manager: waiting on semaphore\n");
		semop(semid, &sop, 1);
		printf("manager: locked on semaphore\n");

		struct shmstruct* shm = (struct shmstruct*) shmat(shmid, NULL, 0);
		if(shm == (void*) -1) {
			printf("manager: shmat failed\n");
		}
		printf("manager: shm attached\n");
		printf("manager: shm.value = {%d}\n", shm -> value);
		printf("manager: shm.message = {%s}\n", shm -> message);

		// shm -> message = strdup("manager.c");
		// asprintf(&(shm->message), "manager.c");
		strcpy(shm -> message, "manager.c");
		shm -> value = 100 + i;
		sleep(1);
		shmdt(shm);

		sop.sem_num = 0;
		sop.sem_op = 1;
		sop.sem_flg = 0;
		printf("manager: releasing semaphore\n");
		semop(semid, &sop, 1);
		printf("manager: released semaphore\n");

	}

	semctl(semid, 0, IPC_RMID, NULL);
	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
