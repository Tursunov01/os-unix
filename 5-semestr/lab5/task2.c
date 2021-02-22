#include <stdio.h>
#include <stdlib.h> // rand
#include <unistd.h> // sleep
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>

//объявление семафор
#define SEM_AW	0
#define SEM_WW	1
#define SEM_AR	2
#define SEM_WR	3

#define SEM_N 4

#define INC  1
#define DEC -1
#define SLP  0

struct sembuf start_read[] = {
    { SEM_WR, INC, 0 },
    { SEM_WW, SLP, 0 },
    { SEM_AW, SLP, 0 },
    { SEM_WR, DEC, 0 },
    { SEM_AR, INC, 0 }
};

struct sembuf stop_read[] = {
    { SEM_AR, DEC, 0 }
};

struct sembuf start_write[] = {
    { SEM_WW, INC, 0 },
    { SEM_AW, SLP, 0 },
    { SEM_AR, SLP, 0 },
    { SEM_WW, DEC, 0 },
    { SEM_AW, INC, 0 }
};

struct sembuf stop_write[] = {
    { SEM_AW, DEC, 0 }
};

#define WRITERS 4
#define READERS 4

#define SEM_COUNT(a) (sizeof(a) / sizeof(struct sembuf))

void writer(int semid, int* const shm, int num) {
    while (1) {

        semop(semid, start_write, SEM_COUNT(start_write));

        (*shm)++;
		printf("Writer #%d write %d\n", num, *shm);

        semop(semid, stop_write, SEM_COUNT(stop_write));

		sleep(rand() % 5);

    }
}

void reader(int semid, int* const shm, int num) {
    while (1) {
        semop(semid, start_read, SEM_COUNT(start_read));

		printf("\t\t\tReader #%d read %d\n", num, *shm);

        semop(semid, stop_read, SEM_COUNT(stop_read));

		sleep(rand() % 5);

    }
}

void make_reader(int reader_id, int sem_id, int* const shm_buff) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Reader's fork error.\n"); 
        perror("Terminating..\n");
        kill(0, SIGKILL);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        reader(sem_id, shm_buff, reader_id);
        exit(EXIT_SUCCESS);
    }
}

void make_writer(int writer_id, int sem_id, int* const shm_buf) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Writer's fork error.\n"); 
        perror("Terminating..\n");
        kill(0, SIGKILL);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        writer(sem_id, shm_buf, writer_id);
        exit(EXIT_SUCCESS);
    }
}

int main() {

    srand(time(NULL));

    int parent_pid = getpid();
  	printf("Parent pid: %d\n", parent_pid);

    int shm_id;
    if ((shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
		perror("!!! Unable to create a shared area.\n");
		exit(EXIT_FAILURE);
	}

    int *shm_buf = shmat(shm_id, 0, 0); 
    if (shm_buf == (int*)-1) {
        perror("!!! Can't attach memory");
        exit(EXIT_FAILURE);
    }

    (*shm_buf) = 0;

    int sem_id;
    if ((sem_id = semget(IPC_PRIVATE, SEM_N, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO)) == -1) 
	{
		perror("!!! Unable to create a semaphore.\n");
		exit(EXIT_FAILURE);
	}

    if (semctl(sem_id, SEM_AW, SETVAL, 0) == -1 ||
        semctl(sem_id, SEM_WW, SETVAL, 0) == -1 ||
        semctl(sem_id, SEM_AR, SETVAL, 0) == -1 ||
        semctl(sem_id, SEM_WR, SETVAL, 0) == -1) {
		perror( "!!! Can't set control semaphors." );
		exit(EXIT_FAILURE);
	}


    for (int i = 0; i < WRITERS; i++) {
        make_writer(i, sem_id, shm_buf);
    }

	for (int i = 0; i < READERS; i++) {
        make_reader(i, sem_id, shm_buf);
	}

    if (shmdt(shm_buf) == -1) {
        perror( "!!! Can't detach shared memory" );
        exit(EXIT_FAILURE);
    }

    int *status = NULL;
    for (int i = 0; i < WRITERS + READERS; i++) {
	    wait(status);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror( "!!! Can't free memory!" );
        exit(EXIT_FAILURE);
    }

    return 0;
}