#include <stdio.h> //printf
#include <stdlib.h> //exit


int main()
{
	int child1 = fork();
	if ( child1 == -1 )
	{
		perror("couldn't fork.");
		exit(1);
	}
	if ( child1 == 0 )
	{
		//потомственный код
		sleep(1);
		printf( "Child1: pid=%d;	group=%d;	parent=%d\n", getpid(), getpgrp(), getppid() );

		return 0;
	}
	int child2 = fork();
	if ( child2 == -1 )
	{
		perror("couldn't fork.");
		exit(1);
	}
	if ( child2 == 0 )
	{
		//потомственный код
		sleep(1);
		printf( "Child2: pid=%d;	group=%d;	parent=%d\n", getpid(), getpgrp(), getppid() );

		return 0;
	}
	else
	{
		//родительский код
        printf( "Parent: pid=%d;	group=%d;	child1=%d;   child2=%d\n", getpid(), getpgrp(), child1, child2 );
		return 0;
	}
}