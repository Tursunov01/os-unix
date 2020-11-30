#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/wait.h> 


int main()
{
	int child = fork();
	if ( child == -1 )
	{
		perror("Couldn't fork.");
		exit(1);
	}
	if ( child == 0 )
	{
		//потомственный код
		sleep(2);
		printf( "Child: pid=%d;	group=%d;	parent=%d\n", getpid(), getpgrp(), getppid() );
	}
	else
	{
		//родительский код
		printf( "Parent: pid=%d;	group=%d;	child=%d\n", getpid(), getpgrp(), child );
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