#include <stdio.h>
#include <sys/stat.h>
#include <pthread.h>

void *write_to_file(int *filenum)
{
    struct stat statbuf;

    FILE *fs = fopen("writers.txt", "w");
    stat("writers.txt", &statbuf);
    printf("+ FOPEN FS%d: inode  = %ld, buffsize = %ld blocksize= %ld\n", *filenum, 
        (long int)statbuf.st_ino, 
        (long int)statbuf.st_size,
        (long int)statbuf.st_blksize);

    for (char c = 'a'; c <= 'z'; c++)
    {
        if (c % 2 && *filenum == 1) //acegi...
            fprintf(fs, "%c", c);

        if (!(c % 2) && *filenum == 2) //bdfh...
            fprintf(fs, "%c", c);
    }

    fclose(fs);
    stat("writers.txt", &statbuf);
    printf("+ FCLOSE FS%d: inode  = %ld, buffsize = %ld blocksize= %ld\n", *filenum, 
        (long int)statbuf.st_ino, 
        (long int)statbuf.st_size,
        (long int)statbuf.st_blksize);
}

int main()
{
    pthread_t thread1;
    pthread_t thread2;

    int num1 = 1;
    int num2 = 2;

    if (pthread_create(&thread2, NULL, write_to_file, &num2) != 0)
    {
        printf("Error: can't create thread 2\n");
        return -1;
    }

    if (pthread_create(&thread1, NULL, write_to_file, &num1) != 0)
    {
        printf("Error: can't create thread 1\n");
        return -1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}