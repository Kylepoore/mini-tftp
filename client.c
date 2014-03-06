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
#include "packet.h"

const char *mode_netascii = "netascii\0";

void send_rrq(int sockfd, struct addrinfo *servinfo, char *fn) {
  char buf[MAXBUFLEN];
  if (pack_rrq(buf, fn, mode_netascii) == -1) {
    perror("send_rrq: pack_rrq");
    exit(EXIT_FAILURE);
  }

  if (sendto(sockfd, buf, MAXBUFLEN, 0, 
             servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    perror("send_rrq: sendto");
    exit(EXIT_FAILURE);
  }
}

void start_reader(struct addrinfo *servinfo) {
  tftp_state client_r = setup_fsm_client_r();

}


void startClient(char *port, char *filename, char *host, 
                 char clientMode){
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

  switch(clientMode) {
    case 'r':
      send_rrq(sockfd, servinfo, filename);
      start_reader(servinfo);
      break;
    case 'w':
      //start_writer(servinfo);
      break;
    default:
      exit(EXIT_FAILURE);
  }


  freeaddrinfo(servinfo);
}
