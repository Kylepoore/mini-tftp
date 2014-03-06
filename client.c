/*
** client.c
**
** Author: Robert Correiro
** Created: March 3,2014
**
*/

#include "client.h"
#include "tftp.h"
#include "fsm.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>

void startClient(char *port, char *filename, char *host, 
                 char clientMode, int verbose){
  printf("client started\n");

  struct addrinfo hints, *servinfo;
  int sockfd, status;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // Don't care if IPv4 or IPv6
  hints.ai_socktype = SOCK_DGRAM; 

  // Resolves IP addr for given host
  if ((status = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
  }

  if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, 
       servinfo->ai_protocol)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    close(sockfd);
    perror("connect");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(servinfo);
}
