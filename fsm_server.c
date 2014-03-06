/*
** fsm_server.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"
#include "packet.h"

tftp_state setup_fsm_server(){
  tftp_state state = {.state = WAITING, .block = -1};
  return state;
}

send_req update_fsm_server(tftp_state *state, struct sockaddr_in address, char *buf){
  send_req request;
  int opcode = getOpCode(buf);
  int block = -1;
  switch(opcode){
    case RRQ :
      vprintf("Read packet received\n");
      break;
    case WRQ :
      vprintf("Write packet received\n");
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
  char *databuf = "hello watson!";
  char *sendBuf = calloc(MAXBUFLEN,sizeof(char));
  request.op = DATA;
  request.length = pack_data(sendBuf,0,databuf,strlen(databuf));
  request.address = address;
  request.buf = sendBuf;


  return request;
}



