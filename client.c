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
#include <signal.h>

const char *mode_octet = "octet\0";
volatile int client_busy = 0;
volatile int client_done = 0;

void stop_client() {
  if (client_busy && !client_done) {
    fprintf(stderr, "\nClient is busy, will shutdown shortly.\n");
    fprintf(stderr, "\nPress ^C to force shutdown.\n");
    client_done = 1;
  } else {
    fprintf(stderr, "\nClient shutdown.\n");
    exit(EXIT_FAILURE);
  }
}

void send_rrq(int sockfd, struct addrinfo *servinfo, char *fn) {
  char buf[MAXBUFLEN];
  if (pack_rrq(buf, fn, mode_octet) == -1) {
    perror("send_rrq: pack_rrq");
    exit(EXIT_FAILURE);
  }

  if (sendto(sockfd, buf, MAXBUFLEN, 0, 
             servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    perror("send_rrq: sendto");
    exit(EXIT_FAILURE);
  }
}


void start_reader(int sockfd, struct addrinfo *servinfo, char *fn) {
  unsigned int bytes, offset;
  char buf[MAXBUFLEN];
  FILE *fp = NULL;

  if ((fp = fopen(fn, "w")) == NULL) {
    perror("start_reader: fopen");
    exit(EXIT_FAILURE);  
  }

  tftp_state client = setup_client(fp);
  
  while (!client_done) {
    if ((bytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
                          servinfo->ai_addr, &servinfo->ai_addrlen)) == -1) {
      perror("start_reader: recvfrom");
    exit(EXIT_FAILURE);
    }

    vprintf("Received packet is %d bytes long.\n", bytes);
    vprintf("Packet contains \"%s\"\n", buf);

    client_busy++;

    // Build the response
    send_req request = build_req(&client, *(servinfo->ai_addr), buf, bytes);
    
    // Error packet received, terminate connection.
    if (request.op == -1) {
      free_send_req(request);
      client_busy--;
      return;
    }

    // Or If something was wrong with the packet, ignore it.
    if (request.op == 0) {
      free_send_req(request);  
      client_busy--;
      continue;
    } 

    // Otherwise, send the response.
    send_packet(sockfd, request);
    free_send_req(request);
    client_busy--;
  }
}


void startClient(char *port, char *filename, char *host, char clientMode) {
  printf("client started\n");

  struct addrinfo hints, *servinfo;
  int sockfd, status;

  signal(SIGINT, stop_client);

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
      start_reader(sockfd, servinfo, filename);
      break;
    case 'w':
      //start_writer(servinfo);
      break;
    default:
      exit(EXIT_FAILURE);
  }

  close(sockfd);
  freeaddrinfo(servinfo);
}
