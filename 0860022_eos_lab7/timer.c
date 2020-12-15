/*
* timer .c
*/
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
void timer_handler(int signum)
{
    static int count = 0;
    printf( "timer expired %d times \n" , ++count);
}
void timer_function(char* pid)
{
    struct sigaction sa;
    struct itimerval timer;

    printf("%s \n",pid);

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
int main(int argc, char **argv)
{
    char* pid_number;
    pid_number = argv[1];
    timer_function(pid_number);
    return 0;
}