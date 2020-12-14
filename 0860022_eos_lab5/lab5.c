#include <stdio.h>  // perror()
#include <stdlib.h> // exit()
#include <fcntl.h>  // open()
#include <signal.h> // signal()
#include <unistd.h> // dup2()

#include <sys/wait.h>

#include "socket_utils.h"
#define BUFSIZE 1024

volatile int stop = 0;
void intHandler(int signum);
void chldHandler(int signum);

int main(int argc, char **argv)
{
  int conn_fd, sock_fd;
  // int n;
  pid_t child_pid;
  struct sockaddr_in cln_addr;
  socklen_t sLen = sizeof(cln_addr);
  // char rcv[BUFSIZE];

  if (argc != 2)
  {
    printf("Usage: %s <port>\n", argv[0]);
    exit(-1);
  }

  sock_fd = createServerSock(atoi(argv[1]), "tcp");
  if (sock_fd < 0)
  {
    perror("Error create socket\n");
    exit(-1);
  }
  signal(SIGINT, intHandler);
  // signal(SIGCHLD, chldHandler);

  while (!stop)
  {
    conn_fd = accept(sock_fd, (struct sockaddr *)&cln_addr, &sLen);
    if (conn_fd == -1)
    {
      perror("Error: accept()");
      continue;
    }

    // No need to read any message
    // n = read(conn_fd, rcv, BUFSIZE);
    // printf("size = %d\n", n);
    // printf("%.*s\n", n, rcv); // print string with given length

    child_pid = fork();
    if (child_pid >= 0)
    {
      if (child_pid == 0)
      {
        dup2(conn_fd, STDOUT_FILENO);
        close(conn_fd);
        execlp("sl", "sl", "-l", NULL);
        exit(-1);
      }
      else
      {
        printf("Train ID: %d\n", (int)child_pid);
      }
    }
  }
  close(sock_fd);
  return 0;
}

void intHandler(int signum)
{
  stop = 1;
}

void chldHandler(int signum)
{
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}