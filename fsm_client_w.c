/*
** fsm_client_w.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"


tftp_state setup_fsm_client_w(){
  tftp_state state = {.state = SENDING, .block = -1};
  return state;
}

tftp_state update_fsm_client_w(tftp_state state, char *buf){










}



