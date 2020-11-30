#include <stdio.h> //printf
#include <stdlib.h> //exi
#include <unistd.h> //pipe
#include <string.h> //strlen
#include <signal.h>
#include <time.h>

int parent_flag = 0;

void sigint_catcher(int signum){
    printf( "\nProccess Catched signal #%d\n", signum);
    printf("Sent secret email to  son!\n");
    parent_flag = 1;
}

int main()
{
	int child;
	int descr[2]; //дескриптор _одного_ программного канала
	signal(SIGINT, sigint_catcher);
	if ( pipe(descr) == -1){
        perror( "Couldn't pipe." );
		exit(1);
	}
	child = fork();
	if ( child == -1 ){
        perror( "Couldn't fork." );
		exit(1);
	}
	if ( child == 0 ){
		close( descr[1] ); //потомок ничего не запишет в канал
		char msg[64];
		memset( msg, 0, 64 );
		int i = 0;
		while( read(descr[0], &(msg[i++]), 1) != '\0' ) ;
		printf("Child: reading..\n\n");
		printf( "Child: read <%s>\n", msg );
	}
	else{
		close( descr[0] ); //предок ничего не считает из канала
		printf( "Parent: waiting for CTRL+C signal for 3 seconds...\n" );
		sleep(3);
		 if (parent_flag){
		     char msg[64] = "It`s my secret email for you, son. Father.";
		     write( descr[1], msg, strlen(msg) ); //передаём сообщение в канал
		     exit(0);
		 }
		 else{
		     char msg[64] = "Hello my child!";
		     write( descr[1], msg, strlen(msg) ); //передаём сообщение в канал
		 }
		return 0;
	}
}