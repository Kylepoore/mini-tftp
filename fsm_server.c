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
  tftp_state state = {.state = WAITING, .block = 0};
  return state;
}

void handle_error(char *buf){
  if(verbose){
    fputs(buf+4,stderr);
  }
}

int update_fsm_server(send_req *request, tftp_state *serverState, struct sockaddr_in address, char *buf, unsigned int bytes){
  request->op = 0;
  char databuf[512];
  unsigned int opcode = getOpCode(buf);
  unsigned short block = 0;
  int length = 0;
  int datalen = 0;
  vprintf("got packet from %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
  request->TID = ntohs(address.sin_port);  
  switch(serverState->state){
    case WAITING :
      switch(opcode){
        case RRQ :
          serverState->TID = ntohs(address.sin_port);
          serverState->done = 0;
          // open a file
          if(access(buf + 2, F_OK) != -1){
            vprintf("entering SENDING state\n");
            serverState->state = SENDING;
            serverState->wait_time = time(NULL);
            serverState->fp = fopen(buf + 2,"r");
            datalen = fread(databuf,sizeof(char),512,serverState->fp);
            length = pack_data(request->buf,1,databuf,datalen);
            serverState->done = datalen < 512;
            serverState->block = 1;
            request->op = DATA;
          }else{
            length = pack_error(request->buf,FILE_NOT_FOUND,"error: file not found\n");
            request->op = ERROR;
          }
          break;
        case WRQ :
          serverState->TID = ntohs(address.sin_port);
          serverState->done = 0;
          if(access(buf + 2, F_OK) != -1){
            vprintf("error: file already exists");
            length = pack_error(request->buf,FILE_EXISTS,"error: file already exists\n");
            request->op = ERROR;
          }else{
            vprintf("entering RECEIVING state\n");
            serverState->state = RECEIVING;
            serverState->wait_time = time(NULL);
            serverState->fp = fopen(buf + 2,"w");
            // send an ack packet
            vprintf("Sent ACK 0\n");
            length = pack_ack(request->buf, 0);
            request->op = ACK;
            serverState->block = 0;
          }
          break;
        case ERROR :
          handle_error(buf);
          break;
        default:
          vprintf(UNEXPECTED);
          // send an error packet
          length = pack_error(request->buf,UNDEFINED,"tftp: unexpected packet\n");
          request->op = ERROR;
      }
      break;
    case SENDING :
      switch(opcode){
        case ACK :
          if(getBlockNo(buf) != serverState->block){
            length = pack_error(request->buf,UNDEFINED,"tftp: wrong block number\n");
            request->op = 0;
          }else{
            vprintf("packet acknowledged\n");
            if(serverState->done){
              vprintf("entering SHUTDOWN state\n");
              serverState->state = SHUTDOWN;
              request->op = 0;
              request->length = 0;
              fclose(serverState->fp);
              break;
            }
            // send another data packet
            serverState->wait_time = time(NULL);
            datalen = fread(databuf, sizeof(char), 512, serverState->fp);
            length = pack_data(request->buf,++(serverState->block),databuf,datalen);
            request->op = DATA;
            if(datalen < 512){
              serverState->done = 1;
            }
          }
          break;
        case ERROR :
          handle_error(buf);
          vprintf("entering SHUTDOWN state\n");
          serverState->state = SHUTDOWN;
          fclose(serverState->fp);
          break;
        default :
          vprintf(UNEXPECTED);
          vprintf("entering SHUTDOWN state\n");
          serverState->state = SHUTDOWN;
          // send an error packet
          length = pack_error(request->buf,UNDEFINED,"tftp: unexpected packet\n");
          request->op = ERROR;
          fclose(serverState->fp);
      }
      break;
    case RECEIVING :
      switch(opcode){
        case DATA :
          vprintf("received data block %d!\n", getBlockNo(buf));
          vprintf("serverState->block + 1 = %d\n", serverState->block);
          if(getBlockNo(buf) != (unsigned short)(serverState->block + 1)){
            length = pack_error(request->buf,UNDEFINED,"tftp: wrong block number\n");
            request->op = 0;
          }else{
            if(fwrite(buf + 4, sizeof(char), bytes - 4, serverState->fp) < bytes - 4){
              length = pack_error(request->buf,DISK_FULL,"tftp: disk is full\n");
              request->op = ERROR;
              vprintf("entering SHUTDOWN state\n");
              serverState->state = SHUTDOWN;
            }else{
              // send ack packet
              vprintf("Sending ACK %d\n", getBlockNo(buf));
              length = pack_ack(request->buf,++(serverState->block));
              serverState->wait_time = time(NULL);
              request->op = ACK;
              
              if(bytes-4 < 512){
                fclose(serverState->fp);
                serverState->state = SHUTDOWN;
              }
            }
          }
          break;
        case ERROR :
          handle_error(buf);
          fclose(serverState->fp);
          break;
        default :
          vprintf(UNEXPECTED);
          vprintf("entering SHUTDOWN state\n");
          serverState->state = SHUTDOWN;
          // send an error packet
          length = pack_error(request->buf,UNDEFINED,"tftp: unexpected packet\n");
          request->op = ERROR;
          handle_error(buf);
          fclose(serverState->fp);
      }
      break;
    case SHUTDOWN :
      vprintf("reached SHUTDOWN state....\n");      
      break;
    default :
      
      break;
  }

  request->length = length;
  request->address = *((struct sockaddr *) &address);
  return 0;
}



