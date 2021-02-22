#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define RUN_ACCESS 0
#define BUF_EMPTY 1
#define BUF_FULL 2

#define BUF_SIZE 10

const int PROD_MAX_NUM = 3;
const int CONS_MAX_NUM = 5;

void pexit(const char *, int err_code);
void create_processes();
void wait_childs();
void clean_resources();

unsigned short init_value[] = {1, BUF_SIZE, 0};

struct sembuf prod_p[2] = {{BUF_EMPTY, -1, 0}, {RUN_ACCESS, -1, 0}};
struct sembuf prod_v[2] = {{BUF_FULL, 1, 0}, {RUN_ACCESS, 1, 0}};
struct sembuf cons_p[2] = {{BUF_FULL, -1, 0}, {RUN_ACCESS, -1, 0}};
struct sembuf cons_v[2] = {{BUF_EMPTY, 1, 0}, {RUN_ACCESS, 1, 0}};

int shm_id = -1;
int sem_id = -1;

char *mem_ptr = NULL;
char *buf = NULL;
char *proc_n = NULL;
char *cons_n = NULL;
char *value = NULL;

int childs = 0;

void producer(int id)
{
    while (1) {
        semop(sem_id, prod_p, 2);
        buf[*proc_n] = *value;
        printf("PRODUCER  %d   pos %d -----> produced %c\n", id, *proc_n, buf[*proc_n]);
        if (*value == 'z'){
            *value = 'a';
        } else {
            *value = *value + 1;
        }
        
        if (*proc_n == BUF_SIZE - 1){
			*proc_n = 0;
		}
		else{
			*proc_n  = *proc_n + 1;
		}
        semop(sem_id, prod_v, 2);

        sleep((unsigned int) (rand() % 3)); 
    }
}

void consumer(int id)
{
    while (1) {
        semop(sem_id, cons_p, 2);
		printf("CONSUMER  %d   pos %d <----- cunsumed %c\n", id, *cons_n, buf[*cons_n]);
		if (*cons_n == BUF_SIZE - 1){
			*cons_n = 0;
		}
		else{
			*cons_n  = *cons_n + 1;
		}
        semop(sem_id, cons_v, 2);

        sleep(10); 
    }
}

int main(void)
{
    int perms = IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO;

    atexit(clean_resources);

    value = mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *value = 'a';

    if ((shm_id = shmget(IPC_PRIVATE, (BUF_SIZE + 2) * sizeof(char), perms)) == -1) {
        pexit("shmget", 1);
    }

    if ((mem_ptr = shmat(shm_id, NULL, 0)) == (char *) -1) {
        pexit("shmat", 2);
    }
    
    proc_n = mem_ptr;
    cons_n = mem_ptr + sizeof(char);
    buf = mem_ptr + 2 * sizeof(char);

    *proc_n = *cons_n = 0;
	

    if ((sem_id = semget(IPC_PRIVATE, 3, perms)) == -1) {
        pexit("semget", 4);
    }

    if (semctl(sem_id, 0, SETALL, init_value) == -1) {
        pexit("semctl", 5);
    }

    create_processes();
    wait_childs();

    return 0;
}

void pexit(const char *msg, int err_code)
{
    perror(msg);
    exit(err_code);
}

void create_processes()
{
    for (int i = 0; i < PROD_MAX_NUM; i++) {
        pid_t pid = fork();
        if (pid < 0) 
			pexit("fork", 1);
        if (!pid) {
            producer(i + 1);
        } else {
            childs++;
        }
    }

    for (int i = 0; i < CONS_MAX_NUM; i++) {
        pid_t pid = fork();
        if (pid < 0) 
			pexit("fork", 1);
        if (!pid) {
            consumer(i + 1);
        } else {
            childs++;
        }
    }
}

void wait_childs()
{
    while (childs--) 
		wait(NULL);
}

void clean_resources()
{
    if (mem_ptr != (void *) -1) shmdt(mem_ptr);
    if (sem_id >= 0) semctl(sem_id, 0, IPC_RMID);
    if (shm_id >= 0) shmctl(shm_id, IPC_RMID, 0);
    if (value != NULL) munmap(value, sizeof(int));
}