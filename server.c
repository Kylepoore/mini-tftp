/*
** server.c
**
** Author: Kyle Poore
** Created: March 3,2014
**
*/

#include "server.h"
#include "tftp.h"
#include "fsm.h"
#include "packet.h"
#include <signal.h>

volatile sig_atomic_t stop = 0;
volatile sig_atomic_t busy = 0;

void stopServer() {
  if(busy && !stop){
    fprintf(stderr,"\nServer is busy, server will shutdown when done.\n");
    fprintf(stderr,"Press ^C again to force shutdown.\n");
    stop = 1;
  }else{
    fprintf(stderr,"\nGoodbye.\n");
    exit(EXIT_SUCCESS);
  }
}

void startServer(char *port) {
  printf("server mode\n");
  int sockfd;
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  unsigned int addr_len, numbytes;
  char buf[MAXBUFLEN];
  int portnum = atoi(port);
  int opcode = 0;
  tftp_state serverState = setup_fsm_server();
  signal(SIGINT,stopServer);
  
  if((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
	
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(portnum);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(my_addr.sin_zero), '\0', 8);

  if(bind(sockfd, (struct sockaddr *) & my_addr, sizeof(struct sockaddr)) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }
	
  while(!stop) { 
    addr_len = sizeof(struct sockaddr);
   	if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *) & their_addr, &addr_len)) == -1){
	    perror("recvfrom");
  	  exit(EXIT_FAILURE);
    }
    //server is busy!! please don't interrupt here!!

    busy++;
    send_req request = update_fsm_server(&serverState, their_addr, buf);
    send_packet(sockfd, request);
    free_send_req(request);
    busy--;

  }
}
