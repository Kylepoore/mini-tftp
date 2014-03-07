tftp_state setup_fsm_server();
int update_fsm_server(send_req *request, tftp_state *state, struct sockaddr_in address, char *buf, unsigned int bytes);

tftp_state setup_client(protocol_state st);
int build_req(send_req *request, tftp_state *client, 
  struct sockaddr address, char *fn, char *buf, unsigned int bytes);
