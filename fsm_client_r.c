/*
** fsm_client_r.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"


tftp_state setup_fsm_client_r(){
  tftp_state state = {.state = RECEIVING, .block = -1};
  return state;
}

tftp_state update_fsm_client_r(tftp_state state, char *buf){
  //todo:









}



