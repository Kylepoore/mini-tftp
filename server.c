/*
** server.c
**
** Author: Kyle Poore
** Created: March 3,2014
**
*/

#include "server.h"
#include "tftp.h"
#include <signal.h>

volatile int stop = 0;
int busy = 0;

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

int getOpCode(char *buf) {
  return ((int)buf[0] << 8) + buf[1];
}

void startServer(char *port, int verbose) {
  printf("server mode\n");
  int sockfd;
  struct sockaddr_in my_addr;
  struct sockaddr_in their_addr;
  unsigned int addr_len, numbytes;
  char buf[MAXBUFLEN];
  int portnum = atoi(port);
  int opcode = 0;
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
    busy = 1;
    char serverMode = 0;
    //added to get source port number
    vprintf("got packet from %s, port %d\n", 
      inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
    vprintf("packet is %d bytes long\n", numbytes);
    //buf[numbytes] = '\0';
    //vprintf("packet contains \"%s\"\n", buf);
    opcode = getOpCode(buf);
    switch(opcode){
      case RRQ :
        vprintf("Read packet received\n");
        serverMode = 'r';
        break;
      case WRQ :
        vprintf("Write packet received\n");
        serverMode = 'w';
        break;
      case DATA :
        vprintf("Data packet received\n");
        break;
      case ACK :
        vprintf("Acknowledgement packet received\n");
        break;
      case ERROR :
        vprintf("Error packet received\n");
        break;
      default :
        vprintf("Malformed packet received\n");
    }
    
    {
      //return the packet to sender
      int bytes_sent;
  	  if((bytes_sent = sendto(sockfd, buf, strlen(buf), 0, 
          (struct sockaddr *) & their_addr, sizeof(struct sockaddr))) == -1){
        perror("send");
       	exit(EXIT_FAILURE);
      }

      vprintf("packet sent, %d bytes\n", bytes_sent);
    }

    busy = 0;
  }
}