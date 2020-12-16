#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include "parking_lot.hpp"
#include "socket_utils.h"

#include <pthread.h> // pthread_create, pthread_join, etc.

void *th(void *arg);

int main(int argc, char **argv)
{
  int sock_fd;
  pthread_t thread;

  if (argc != 2)
  {
    printf("Usage: %s <port>\n", argv[0]);
    exit(-1);
  }
  sock_fd = createServerSock(atoi(argv[1]), TRANSPORT_TYPE_TCP);

  ParkingLot pl(sock_fd);

  if (pthread_create(&thread, NULL, th, (void *)&pl))
  {
    perror("Error: pthread_create()\n");
  }
  while (1)
  {
    pl.runSystem();
  }
}

void *th(void *arg)
{
  char msg_buf[MSG_BUFSIZE];
  ParkingLot *pl = (ParkingLot *)arg;
  while (scanf("%s", msg_buf))
  {
    printf("%s\n", msg_buf);
    if (strcmp(msg_buf, "end") == 0)
    {
      printf("called\n");
      pl->logStatus("result.txt");
      break;
    }
  }
  return (void *)NULL;
}
