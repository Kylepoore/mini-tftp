/*
** fsm_client_r.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"
#include "packet.h"

tftp_state setup_fsm_client_r(){
  tftp_state state = {.state = RECEIVING, .block = -1};
  return state;
}

send_req update_fsm_client_r(tftp_state *state, struct sockaddr_in address, char *buf){
  //todo:









}



