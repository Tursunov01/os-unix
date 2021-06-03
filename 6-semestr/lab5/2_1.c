#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *read_file(int *fd)
{
    char c;
    while (read(*fd, &c, 1) == 1)
    {
        write(1, &c, 1);
    }

    sleep(rand() % 2);
}

int main()
{
    int fd1 = open("alphabet.txt", O_RDONLY);
    int fd2 = open("alphabet.txt", O_RDONLY);

    pthread_t thread1;
    pthread_t thread2;

    int status1 = pthread_create(&thread1, NULL, read_file, &fd1);
    if (status1 != 0)
    {
        printf("Error: can't create thread 1\n");
        return -1;
    }

    int status2 = pthread_create(&thread2, NULL, read_file, &fd2);
    if (status2 != 0)
    {
        printf("Error: can't create thread 2\n", status2);
        return -1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}