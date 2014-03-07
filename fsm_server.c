/*
** fsm_server.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"
#include "packet.h"

#define UNEXPECTED "unexpected packet!\n"

tftp_state setup_fsm_server(){
  tftp_state state = {.state = WAITING, .block = -1};
  return state;
}

send_req update_fsm_server(tftp_state *serverState, struct sockaddr_in address, char *buf, unsigned int bytes){
  send_req request = {.op = 0};
  char databuf[512];
  int opcode = getOpCode(buf);
  int block = -1;
  int length = 0;
  char *sendBuf = calloc(MAXBUFLEN,sizeof(char));
  int datalen = 0;
  vprintf("got packet from %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
  
  switch(serverState->state){
    case WAITING :
      switch(opcode){
        case RRQ :
          vprintf("entering SENDING state\n");
          // open a file
          if(access(buf + 2, F_OK) != -1){
            serverState->state = SENDING;
            serverState->fp = fopen(buf + 2,"r");
            datalen = fread(databuf,sizeof(char),512,serverState->fp);
            length = pack_data(sendBuf,1,databuf,datalen);
            serverState->block = 1;
            request.op = DATA;
          }else{
            length = pack_error(sendBuf,FILE_NOT_FOUND,"tftp: file not found\n");
            request.op = ERROR;
          }
          break;
        case WRQ :
          vprintf("entering RECEIVING state\n");
          if(access(buf + 2, F_OK) != -1){
            length = pack_error(sendBuf,FILE_EXISTS,"tftp: file already exists\n");
            request.op = ERROR;
          }else{
            serverState->state = RECEIVING;
            serverState->fp = fopen(buf + 2,"w");
            // send an ack packet
            length = pack_ack(sendBuf, 0);
            request.op = ACK;
          }
          break;
        default:
          vprintf(UNEXPECTED);
          // send an error packet
          length = pack_error(sendBuf,UNDEFINED,"tftp: unexpected packet\n");
          request.op = ERROR;
      }
      break;
    case SENDING :
      switch(opcode){
        case ACK :
          vprintf("packet acknowledged\n");
          // send another data packet
          datalen = fread(databuf, sizeof(char), 512, serverState->fp);
          length = pack_data(sendBuf,serverState->block++,databuf,datalen);
          request.op = DATA;
          if(datalen < 512){
            vprintf("entering WAITING state\n");
            serverState->state = WAITING;
          }
          break;
        default :
          vprintf(UNEXPECTED);
          serverState->state = WAITING;
          // send an error packet
          length = pack_error(sendBuf,UNDEFINED,"tftp: unexpected packet\n");
          request.op = ERROR;
      }
      break;
    case RECEIVING :
      switch(opcode){
        case DATA :
          vprintf("received data!\n");
          if(getBlockNo(buf) != serverState->block + 1){
            length = pack_error(sendBuf,UNDEFINED,"tftp: wrong block number\n");
            request.op = ERROR;
          }else{
            if(fwrite(buf + 4, sizeof(char), bytes - 4, serverState->fp) < bytes - 4){
              length = pack_error(sendBuf,DISK_FULL,"tftp: disk is full\n");
              request.op = ERROR;
              serverState->state = WAITING;
            }else{
              // send ack packet
              length = pack_ack(sendBuf,getBlockNo(buf));
              request.op = ACK;
            }
          }
          break;
        default :
          vprintf(UNEXPECTED);
          serverState->state = WAITING;
          // send an error packet
          length = pack_error(sendBuf,UNDEFINED,"tftp: unexpected packet\n");
          request.op = ERROR;
      }
      break;
    case SHUTDOWN :
      
      break;
    default :
      
      break;
  }

  request.length = length;
  request.address = *((struct sockaddr *) &address);
  request.buf = sendBuf;
  return request;
}



