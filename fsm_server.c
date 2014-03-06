/*
** fsm_server.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"

tftp_state setup_fsm_server(){
  tftp_state state = {.state = WAITING, .block = -1};
  return state;
}

tftp_state update_fsm_server(tftp_state state, char *buf){
  //todo:









}



