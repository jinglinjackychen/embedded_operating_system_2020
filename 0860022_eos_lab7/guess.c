#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHMSZ 27

void timer_handler(int signum)
{
    static int count = 0;
    printf( "timer expired %d times \n" , ++count);
}
void timer_function(int pid)
{
    struct sigaction sa;
    struct itimerval timer;

    printf("%d \n",pid);

    /* Install timer_handler as the signal handler for SIGVTALRM */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after 250 msec */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;

    /* Reset the timer back to 250 msec after expired */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    /* Start a virtual timer */
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    /* Do busy work */
    while (1);
}
void shm_client(int input_key)
{
	int shmid;
	key_t key;
	char *shm, *s;

	/* We need to get the segment named "5678" , created by the server */
	key = input_key;

	/* Locate the segment */
	if((shmid = shmget(key, SHMSZ, 0666)) < 0)
	{
		perror("shmget");
		exit(1);
	}
	
	/* Now we attach the segment to our data space */
	if((shm = shmat(shmid, NULL, 0)) == (char *) -1)
	{
		perror("shmat");
		exit(1);
	}
	printf("Client attach the share memory created by server.  \n");

	/* Now read what the server put in the memory */
	printf("Client read characters from share memory...  \n");
	for(s = shm; *s != '\0' ; s++)
		putchar(*s);
	putchar( '\n' );

	/*
	* Finally , change the first character of the segment to ’*’,
	* indicating we have read the segment .
	*/
	printf("Client write ∗ to the share memory.  \n");
	*shm = '*';

	/* Detach the share memory segment */
	printf(" Client detach the share memory.  \n");
	shmdt(shm);
}

int main(int argc, char **argv)
{
    int key_number,upper_bound,pid_number;
	key_number = atoi(argv[1]);
	upper_bound = atoi(argv[2]);
    pid_number = atoi(argv[3]);

    //timer_function(pid_number);
	shm_client(key_number);
    return 0;
}