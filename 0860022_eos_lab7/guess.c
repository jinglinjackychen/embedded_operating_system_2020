#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHMSZ 27

typedef struct
{
	int guess;
	char result[8];
} data;

void timer_handler(int signum)
{
	static int count = 0;
	printf("timer expired %d times \n", ++count);
}
void timer_function(int pid)
{
	struct sigaction sa;
	struct itimerval timer;

	printf("%d \n", pid);

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
	while (1)
		;
}
void shm_client(int input_key, int upper_bound)
{
	int shmid, check;
	key_t key;
	char *shm, *s;
	data *sh_guess_number, *guess_number;

	int low = 1, high = upper_bound;
	char bingo[8] = "bingo";
	char smaller[8] = "smaller";
	char bigger[8] = "bigger";

	/* We need to get the segment named "5678" , created by the server */
	key = input_key;

	/* Locate the segment */
	if ((shmid = shmget(key, 1, 0666)) < 0)
	{
		perror("shmget");
		exit(1);
	}

	/* Now we attach the segment to our data space */
	if ((sh_guess_number = shmat(shmid, NULL, 0)) == (data *)-1)
	{
		perror("shmat");
		exit(1);
	}
	//printf("Client attach the share memory created by server. \n");

	/* Now read what the server put in theguess_number memory */
	//printf("Client read data from share memory...  \n");
	guess_number = sh_guess_number;
	//printf("%d",guess_number->guess);
	//putchar('\n');

	/*
	* Finally , change the first character of the segment to ’*’,
	* indicating we have read the segment .
	*/
	//printf("Client write data to the share memory. \n");

	while (strcmp(guess_number->result,bingo) != 0)
	{
		check = (low + high) / 2;
		guess_number->guess = check;
		printf("[game] Guess: %d\n",guess_number->guess);

		if(strcmp(guess_number->result,bingo) == 0)
		{
			break;
		}
		else if(strcmp(guess_number->result,smaller) == 0)
		{
			high = guess_number->guess;
		}
		else if(strcmp(guess_number->result,bigger) == 0)
		{
			low = guess_number->guess;
		}
		sleep(1);
	}

	/* Detach the share memory segment */
	//printf("Client detach the share memory. \n");
	shmdt(guess_number);
}

int main(int argc, char **argv)
{
	int key_number, upper_bound, pid_number;
	key_number = atoi(argv[1]);
	upper_bound = atoi(argv[2]);
	pid_number = atoi(argv[3]);

	//timer_function(pid_number);
	shm_client(key_number, upper_bound);
	return 0;
}