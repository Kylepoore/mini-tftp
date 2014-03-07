tftp_state setup_fsm_server();
send_req update_fsm_server(tftp_state *state, struct sockaddr_in address, char *buf, unsigned int bytes);

tftp_state setup_fsm_client_r();
send_req update_fsm_client_r(tftp_state *state, struct sockaddr_in address, char *buf);

tftp_state setup_fsm_client_w();
send_req update_fsm_client_w(tftp_state *state, struct sockaddr_in address, char *buf);
