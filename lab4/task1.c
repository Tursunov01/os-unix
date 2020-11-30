#include <stdio.h> //printf
#include <stdlib.h> //exit


int main()
{
	int child = fork();
	if ( child == -1 )
	{
		perror("couldn't fork.");
		exit(1);
	}
	if ( child == 0 )
	{
		//потомственный код
		sleep(1);
		printf( "Child: pid=%d;	group=%d;	parent=%d\n", getpid(), getpgrp(), getppid() );

		return 0;
	}
	else
	{
		//родительский код
        printf( "Parent: pid=%d;	group=%d;	child=%d\n", getpid(), getpgrp(), child );
		return 0;
	}
}