#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/wait.h> 


int main()
{
	int child1 = fork();
	if ( child1 == -1 )
	{
		perror("Couldn't fork.");
		exit(1);
	}
	if ( child1 == 0 )
	{
		//потомственный код
		sleep(2);
		printf( "Child1: pid=%d;	group=%d;	parent=%d\n", getpid(), getpgrp(), getppid() );
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
		printf( "Parent: pid=%d;	group=%d;	child1=%d;   child2=%d\n", getpid(), getpgrp(), child1, child2);		
		int status;
		pid_t ret_value;
		
		ret_value = wait( &status );
		if ( WIFEXITED(status) )
		    printf("Parent: child %d finished with %d code.\n", ret_value, WEXITSTATUS(status) );
		else if ( WIFSIGNALED(status) )
		    printf( "Parent: child %d finished from signal with %d code.\n", ret_value, WTERMSIG(status));
		else if ( WIFSTOPPED(status) )
		    printf("Parent: child %d finished from signal with %d code.\n", ret_value, WSTOPSIG(status));
		return 0;
	}
}