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

send_req build_req(tftp_state *client, 
  struct sockaddr address, char *buf, unsigned int bytes){

  char req_buf[512];
  int op = getOpCode(buf);
  int block = getBlockNo(buf);
  int length = 0;

  send_req req = {.op = 0};

  switch(client->state) {
    case READER:
      switch(op) {
        case DATA:  
          if (block <= client->block) {
            return req;
          }

          if (fwrite(buf + 4, sizeof(char), bytes - 4, client->fp) < bytes - 4) {
            length = pack_error(req_buf, DISK_FULL, "Error: Disk is full.");
            req.op = ERROR;
            break;
          }

          length = pack_ack(req_buf, block);
          req.op = ACK;
          break;

        case ERROR:
          fclose(client->fp);
          req.op = -1;
          break;

        default:
          break;
      }
      break;

    // case WRITER:
    //   switch(op) {

    //   }
    //   break;

    default:
      break;
  }

  req.address = address;
  req.length = length;
}