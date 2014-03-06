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
  vprintf("got packet from %s, port %d\n", 
    inet_ntoa(address.sin_addr), ntohs(address.sin_port));
  char *databuf;
  int opcode = getOpCode(buf);
  int block = -1;
  switch(opcode){
    case RRQ :
      vprintf("Read packet received\n");
      databuf = "hello read request!";
      break;
    case WRQ :
      vprintf("Write packet received\n");
      databuf = "hello write request!";
      break;
    case DATA :
      vprintf("Data packet received\n");
      databuf = "hello data packet!";
      break;
    case ACK :
      vprintf("Acknowledgement packet received\n");
      databuf = "hello acknowledgement packet!";
      break;
    case ERROR :
      vprintf("Error packet received\n");
      databuf = "hello error packet!";
      break;
    default :
      vprintf("Malformed packet received\n");
      databuf = "hey, who do you think you are??";
  }
  char *sendBuf = calloc(MAXBUFLEN,sizeof(char));
  request.op = DATA;
  request.length = pack_data(sendBuf,0,databuf,strlen(databuf));
  request.address = address;
  request.buf = sendBuf;


  return request;
}



