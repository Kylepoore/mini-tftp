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

void serverThread(int sockfd, struct sockaddr_in their_addr, tftp_state serverState, send_req request){
  struct sockaddr_in my_addr;
  unsigned int addr_len;
  int numbytes;
  char buffer[MAXBUFLEN];
  
    
  addr_len = sizeof(struct sockaddr);

  send_packet(sockfd, request, &serverState);
  
  vprintf("entering server loop\n"); 
  while(serverState.state != SHUTDOWN) {
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

  addr_len = sizeof(struct sockaddr);
  
  while(!stop) { 
   	if((numbytes = recvfrom(sockfd, &buffer, MAXBUFLEN, 0, (struct sockaddr *) & their_addr, &addr_len)) == -1){
	    perror("recvfrom");
  	  exit(EXIT_FAILURE);
    }
    //server is busy!! please don't interrupt here!!
    busy++;
    send_req request;
    tftp_state serverState = setup_fsm_server();
    update_fsm_server(&request, &serverState, their_addr, buffer, numbytes);
    int bytes_sent;
//  if((bytes_sent = sendto(sockfd, "hello1", 7, 0, 
//      (struct sockaddr *) &request.address, sizeof(struct sockaddr))) == -1) {
//    perror("send_packet->sendto");
//    exit(EXIT_FAILURE);
//  }
//
//  if((bytes_sent = sendto(sockfd, "hello2", 7, 0, 
//      (struct sockaddr *) & their_addr, sizeof(struct sockaddr))) == -1) {
//    perror("send_packet->sendto");
//    exit(EXIT_FAILURE);
//  }
//    close(sockfd);
    serverThread(sockfd,their_addr, serverState, request);
    busy--;

  }
}
