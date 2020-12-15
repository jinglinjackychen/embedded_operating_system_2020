#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define SHMSZ 27

typedef struct {
    int guess;
    char result[8];
}data;

void shm_create(int input_key,int input_number)
{
	char c;
	int shmid;
	key_t key;
	data *sh_guess_number,*guess_number;
	int retval;

	/* We ’ll name our shared memory segment "5678" */
	key = input_key;

	/* Create the segment */
	if((shmid = shmget(key, 1, IPC_CREAT | 0666)) < 0)
	{
		perror("shmget");
		exit(1);
	}

	/* Now we attach the segment to our data space */
	if((sh_guess_number = shmat(shmid, NULL, 0)) == (data *) -1)
	{
		perror("shmat");
		exit(1);
	}
	printf("Server create and attach the share memory. \n");

	/* Now put some things into the memory for the other process to read */
	guess_number = sh_guess_number;
	printf("Server write guess_number to share memory. \n");
	guess_number->guess = input_number;

	/*
	* Finally , we wait until the other process changes the first
	* character of our memory to ’*’, indicating that it has read
	* what we put there .
	*/
	printf("Waiting other process read the share memory...  \n");
	while (guess_number->guess != 100)
		sleep(1);
	printf("Server read 100 from the share memory.  \n");
	/* Detach the share memory segment */
	shmdt(sh_guess_number);
	/* Destroy the share memory segment */
	printf("Server destroy the share memory.  \n");
	retval = shmctl(shmid, IPC_RMID, NULL);
	if(retval < 0)
	{
		fprintf(stderr, "Server remove share memory failed \n");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	int key_number,guess;
	key_number = atoi(argv[1]);
	guess = atoi(argv[2]);

	shm_create(key_number,guess);
	return 0;
}
