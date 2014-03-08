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

void serverThread(struct sockaddr_in their_addr, tftp_state serverState, send_req request){
  int sockfd;
  struct sockaddr_in my_addr;
  unsigned int addr_len;
  int numbytes;
  char buffer[MAXBUFLEN];
  
    
  if((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
  
  int flags = fcntl(sockfd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(sockfd, F_SETFL, flags);

  addr_len = sizeof(struct sockaddr);

  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  my_addr.sin_port = 0;
	
  bind(sockfd, (struct sockaddr *)&my_addr,addr_len);
  getsockname(sockfd,(struct sockaddr *) & my_addr,&addr_len);
  int port = ntohs(my_addr.sin_port);
  vprintf("serving connection to %s %d from %s %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), inet_ntoa(my_addr.sin_addr),port); 

  send_packet(sockfd, request, &serverState);
	
  while(serverState.state != SHUTDOWN) {
    vprintf("entered server loop\n"); 
   	numbytes = recvfrom_timeout(sockfd, &buffer, MAXBUFLEN, 0, (struct sockaddr *) & their_addr, &addr_len, serverState);
    vprintf("exited from recvfrom_timeout\n");
  	if(numbytes > 0){    
      update_fsm_server(&request, &serverState, their_addr, buffer, numbytes);
    }
    send_packet(sockfd, request, &serverState);

  }
}





void startServer(char *port) {
  printf("server mode\n");
  int sockfd;
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  unsigned int addr_len, numbytes;
  char buffer[MAXBUFLEN];
  int portnum = atoi(port);
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
   	if((numbytes = recvfrom(sockfd, &buffer, MAXBUFLEN, 0, (struct sockaddr *) & their_addr, &addr_len)) == -1){
	    perror("recvfrom");
  	  exit(EXIT_FAILURE);
    }
    //server is busy!! please don't interrupt here!!
    busy++;
    send_req request;
    tftp_state serverState = setup_fsm_server();
    update_fsm_server(&request, &serverState, their_addr, buffer, numbytes);
    serverThread(their_addr, serverState, request);
    busy--;

  }
}
