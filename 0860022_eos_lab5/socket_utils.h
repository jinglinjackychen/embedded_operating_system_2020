#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <fcntl.h>  // open()
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // struct sockaddr_in
#include <netdb.h>
#include <stdio.h>


int createServerSock(int port, const char* type);
int createClientSock(const char* host, int port, const char* type);


#endif