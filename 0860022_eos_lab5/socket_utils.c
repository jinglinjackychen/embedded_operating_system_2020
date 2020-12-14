#include "socket_utils.h"

int createServerSock(int port, const char *type)
{
  int s, yes=1;
  struct sockaddr_in sin;


  memset(&sin, 0, sizeof(sin)); // Init sin
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons((unsigned short)port);
  

  if (strcmp(type, "tcp") == 0)
  {
    s = socket(PF_INET, SOCK_STREAM, 0);
  }
  else if (strcmp(type, "udp") == 0)
  {
    s = socket(PF_INET, SOCK_DGRAM, 0);
  }
  else
  {
    perror("Wrong transport type. Must be \"udp\" or \"tcp\"\n");
    return -1;
  }

  if (s < 0)
  {
    perror("Can't create socket\n");
    return -1;
  }

  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("Can't bind to port\n");
    return -1;
  }

  if (listen(s, 10) < 0)
  {
    perror("Can'n listen on port\n");
    return -1;
  }
  return s;
}
