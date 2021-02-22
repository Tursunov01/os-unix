#include <stdio.h>
#include <stdlib.h> // rand
#include <unistd.h> // sleep
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>

const int N = 9;
// const int COUNT = 5;
const size_t shm_size = (N + 2) * sizeof(int);

int* shm_attached_address;
char* shm_buffer;
int* shm_pos_consumer;
int* shm_pos_producer;

#define SEM_BINARY 0
#define SEM_EMPTY 1
#define SEM_FULL 2

#define P -1
#define V 1

struct sembuf producer_start[2] = 
    { { SEM_EMPTY , P, 0 }, { SEM_BINARY, P, 0 } };
struct sembuf producer_stop [2] = 
    { { SEM_BINARY, V, 0 }, { SEM_FULL  , V, 0 } };
struct sembuf consumer_start[2] = { 
    { SEM_FULL  , P, 0 }, { SEM_BINARY, P, 0 } };
struct sembuf consumer_stop [2] = 
    { { SEM_BINARY, V, 0 }, { SEM_EMPTY , V, 0 } };

void producer(const int semid, const char value, const int producer_id) {
    sleep(rand() % 3);
    if (semop(semid, producer_start, 2) == -1) {
        puts("PRODUCER   can not make operation on semaphore");
        exit(EXIT_FAILURE);
    }
    shm_buffer[*shm_pos_producer] = value;
    printf("PRODUCER  %d   pos %d -----> produced %c\n", 
        producer_id, *shm_pos_producer, shm_buffer[*shm_pos_producer]);
    (*shm_pos_producer)++;
    if (semop(semid, producer_stop, 2) == -1) {
        puts("PRODUCER   can not make operation on semaphore");
        exit(EXIT_FAILURE);
    }
}

void consumer(const int semid, const char value, const int consumer_id) {
    sleep(rand() % 3);
    if (semop(semid, consumer_start, 2) == -1) {
        puts("PRODUCER   can not make operation on semaphore");
        exit(EXIT_FAILURE);
    }
    printf("CONSUMER  %d   pos %d <----- cunsumed %c\n", consumer_id, *shm_pos_consumer, shm_buffer[*shm_pos_consumer]);
    (*shm_pos_consumer)++;

    if (semop(semid, consumer_stop, 2) == -1) {
        puts("PRODUCER   can not make operation on semaphore");
        exit(EXIT_FAILURE);
    }
}

void make_producer(const int id, const int semid, char value, int COUNT) {
    pid_t child1_pid;
    if ((child1_pid = fork()) == -1) {
        puts("Can't fork");
        exit(EXIT_FAILURE);
    }
    if(child1_pid == 0) {
        // child
        printf("Created producer %d\n", id);
        for (int i = 0; i < COUNT; i++) {
            producer(semid, value, id);
            value++;
        }

        printf("PRODUCER  %d   finished his work. Terminating..\n", id);
        exit(EXIT_SUCCESS);
    }
}

void make_consumer(const int id, const int semid, int COUNT) {
    pid_t child_pid;
    if ((child_pid = fork()) == -1) {
        puts("Can't fork");
        exit(EXIT_FAILURE);
    }
    if(child_pid == 0) {
        printf("Created consumer %d\n", id);
        // child
        for (int i = 0; i < COUNT; i++) {
            consumer(semid, i, id);
        }

        printf("CONSUMER  %d   finished his work. Terminating..\n", id);
        exit(EXIT_SUCCESS);
    }
}

// 3 семафора

int main() {
    srand(0);
    int shmid;  // shared memory id
    int semid;  // semaphore id

    pid_t parent_pid = getpid();
    printf("Parent pid: %i\n", parent_pid);
    shmid = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
    if (shmid == -1) {
        puts("Unable to create shared area");
        exit(EXIT_FAILURE);
    }

    shm_attached_address = shmat(shmid, NULL, 0);

    if (*(char*)shm_attached_address == -1) {
        puts("Can not attach memory");
        exit(EXIT_FAILURE);
    }

    shm_pos_producer = shm_attached_address;
    shm_pos_consumer = shm_attached_address +     sizeof(int);
    shm_buffer       = shm_attached_address + 2 * sizeof(char);
    (*shm_pos_producer) = 0;
    (*shm_pos_consumer) = 0;

    if ((semid = semget(IPC_PRIVATE, 3, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
        puts("Unable to create semapthores");
        exit(EXIT_FAILURE);
    }

    int ctl_sem_binary = semctl(semid, SEM_BINARY, SETVAL, 1);
    int ctl_sem_empty  = semctl(semid, SEM_EMPTY , SETVAL, N);
    int ctl_sem_full   = semctl(semid, SEM_FULL  , SETVAL, 0);

    if (ctl_sem_binary == -1 || ctl_sem_empty == -1 || ctl_sem_full == -1) {
        puts("Cannot set controll semaphores");
        exit(EXIT_FAILURE);
    }
    
    make_consumer(0, semid, 4);
    make_consumer(1, semid, 5);

    make_producer(0, semid, 'a', 3);
    make_producer(1, semid, 'b', 3);
    make_producer(2, semid, 'c', 3);

    int status;
    for (int i = 0; i < 5; i++) {
        wait(&status);
    }

    if (shmdt(shm_attached_address) == -1) { // Detach shared memory segment
        puts("Can't detach shared memory segment");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}