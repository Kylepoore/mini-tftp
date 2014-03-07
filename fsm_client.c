/*
** fsm_client.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"
#include "packet.h"

tftp_state setup_client(FILE *fi){
  tftp_state state = {.block = -1, .fp = fi};
  return state;
}

send_req build_response(tftp_state *client, 
  struct sockaddr_in address, char *buf){

  char data[512];
  int op = getOpCode(buf);
  int block_num = getBlockNo(buf);

  send_req res = {.op = 0};

  switch(client->state) {
    case READER:
      switch(op) {
        case DATA:  
          if (block_num <= client->block) {

          }

        case ERROR:

        case default:
      }
      break;

    case WRITER:
      switch(op) {

      }
      break;

    case default:
  }
}



