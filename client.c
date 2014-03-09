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
volatile int client_busy_sig = 0;
volatile int client_done_sig = 0;

void stop_client() {
  if (client_busy_sig && !client_done_sig) {
    fprintf(stderr, "\nClient is busy, will shutdown shortly.\n");
    fprintf(stderr, "\nPress ^C to force shutdown.\n");
    client_done_sig = 1;
  } else {
    fprintf(stderr, "\nClient shutdown.\n");
    exit(EXIT_FAILURE);
  }
}

void send_rrq(int sockfd, struct addrinfo *servinfo, char *fn) {
  char buf[516];
  if (pack_rrq(buf, fn, mode_octet) == -1) {
    perror("send_rrq: pack_rrq");
    exit(EXIT_FAILURE);
  }

  if (sendto(sockfd, buf, 516, 0, 
             servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    perror("send_rrq: sendto");
    exit(EXIT_FAILURE);
  }
}

void start_reader(int sockfd, struct addrinfo *servinfo, char *fn) {
  int bytes;
  send_req request;
  int status;
  char buf[MAXBUFLEN];

  struct sockaddr_in their_addr;
  int addr_len = sizeof(their_addr);

  tftp_state client = setup_client(INIT);

  // MISSING: Need to account for RRQ packet getting lost
  vprintf("Sending RRQ packet.\n");
  send_rrq(sockfd, servinfo, fn); 
  client.wait_time = time(NULL); 
  vprintf("After send_rrq call.\n");

  while (!client_done_sig) {
    vprintf("Before recvfrom.\n");
    bytes = recvfrom_timeout(sockfd, buf, MAXBUFLEN-1, 0, 
                (struct sockaddr *)&their_addr, &addr_len, client);

    client_busy_sig++;
    if (bytes > 0) {
      vprintf("Received %d bytes.\n", bytes);
      vprintf("got packet from %s %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));

      // Act on received packet and build response
      status = build_req(&request, &client, *((struct sockaddr *)&their_addr), fn, buf, bytes);

      if (client.done) {
        fclose(client.fp);
        client_done_sig = 1;
      }

      switch(status) {
        client_busy_sig--;
        case -1:
          vprintf("Error packet received, terminating.\n");
          return;
        case 0:
          vprintf("Something wrong with packet, ignoring.\n");
          continue;
      } 

      // Otherwise, send the response.
      vprintf("Sending response packet.\n");
    } else {
      vprintf("Resending last packet.\n");
    }

    send_packet(sockfd, request, &client);
    client_busy_sig--;
  }
}

void send_wrq(int sockfd, struct addrinfo *servinfo, char *fn) {
  char buf[516];
  if (pack_wrq(buf, fn, mode_octet) == -1) {
    perror("send_wrq: pack_wrq");
    exit(EXIT_FAILURE);
  }

  if (sendto(sockfd, buf, 516, 0, 
             servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
    perror("send_wrq: sendto");
    exit(EXIT_FAILURE);
  }
}

void start_writer(int sockfd, struct addrinfo *servinfo, char *fn) {
  int bytes;
  send_req request;
  int status;
  char buf[MAXBUFLEN];

  tftp_state client = setup_client(INIT);

  // MISSING: Need to account for WRQ packet getting lost
  vprintf("Sending WRQ packet.\n");
  send_wrq(sockfd, servinfo, fn);  
  client.wait_time = time(NULL);
  
  while (!client_done_sig) {
    bytes = recvfrom_timeout(sockfd, buf, MAXBUFLEN-1, 0, 
                     servinfo->ai_addr, &servinfo->ai_addrlen, client);

    client_busy_sig++;
    if (bytes > 0) {
      vprintf("Received %d bytes.\n", bytes);

      // Act on received packet and build response
      status = build_req(&request, &client, *(servinfo->ai_addr), fn, buf, bytes);

      if (client.done) {
        fclose(client.fp);
        client_done_sig = 1;
      }

      switch(status) {
        client_busy_sig--;

        case -1:
          vprintf("Error packet received, terminating.\n");
          return;

        case 0:
          vprintf("Something wrong with packet, ignoring.\n");
          continue;

        case 2:
          vprintf("Received final ACK.\n");
          return;
      }
    } else {
      vprintf("Resending last packet.\n");
    }

    // Otherwise, send the response.
    send_packet(sockfd, request, &client);
    client_busy_sig--;
  }
}


void startClient(char *port, char *filename, char *host, char clientMode) {
  struct addrinfo hints, *servinfo;
  int sockfd, status;
  struct sockaddr_in my_addr;
  int addr_len;

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

  int flags = fcntl(sockfd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(sockfd, F_SETFL, flags);


  // addr_len = sizeof(struct sockaddr_in);

  // my_addr.sin_family = AF_INET;
  // my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  // my_addr.sin_port = 0;

  // if (bind(sockfd, (struct sockaddr *)&my_addr, addr_len) == -1) {
  //   close(sockfd);
  //   perror("bind");
  //   exit(EXIT_FAILURE);
  // }


  // if (bindconnect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
  //   close(sockfd);
  //   perror("connect");
  //   exit(EXIT_FAILURE);
  // }

  switch(clientMode) {
    case 'r':
      vprintf("Starting reader client.\n");
      start_reader(sockfd, servinfo, filename);
      break;
    case 'w':
      vprintf("Starting writer client.\n");
      start_writer(sockfd, servinfo, filename);
      break;
    default:
      vprintf("Invalid client mode.\n");
      break;
  }

  vprintf("Client done, shutting down.\n");
  close(sockfd);
  freeaddrinfo(servinfo);
}
