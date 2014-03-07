tftp_state setup_fsm_server();
send_req update_fsm_server(tftp_state *state, struct sockaddr_in address, char *buf, unsigned int bytes);

tftp_state setup_client(FILE *fi);
send_req build_req(tftp_state *client, struct sockaddr address, 
  char *buf, unsigned int bytes);
