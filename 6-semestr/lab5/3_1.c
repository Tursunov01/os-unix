#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
int main()
{
    struct stat statbuf;

    FILE *fs1 = fopen("writers.txt", "w");
    stat("writers.txt", &statbuf);
    printf(" + FOPEN FS1: inode  = %ld, buffsize = %ld, blocksize= %ld\n", 
        (long int)statbuf.st_ino, 
        (long int)statbuf.st_size,
        (long int)statbuf.st_blksize);

    FILE *fs2 = fopen("writers.txt", "w");
    stat("writers.txt", &statbuf);
    printf(" + FOPEN FS2: inode  = %ld, buffsize = %ld, blocksize= %ld\n", 
        (long int)statbuf.st_ino, 
        (long int)statbuf.st_size,
        (long int)statbuf.st_blksize);

    for (char c = 'a'; c <= 'z'; c++)
    {
        if (c % 2)
            fprintf(fs1, "%c", c); //aceg...
        else
            fprintf(fs2, "%c", c); //bdfh...
    }

    fclose(fs2);
    stat("writers.txt", &statbuf);
    printf(" + FCLOSE FS2: inode  = %ld, buffsize = %ld, blocksize= %ld\n", 
        (long int)statbuf.st_ino, 
        (long int)statbuf.st_size,
        (long int)statbuf.st_blksize);

    fclose(fs1);
    stat("writers.txt", &statbuf);
    printf(" + FCLOSE FS1: inode  = %ld, buffsize = %ld, blocksize= %ld\n", 
        (long int)statbuf.st_ino, 
        (long int)statbuf.st_size,
        (long int)statbuf.st_blksize);

    return 0;
}
