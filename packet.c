#include "tftp.h"
#include "packet.h"

unsigned int getOpCode(char *buf) {
  return ((unsigned int)buf[0] << 8) + (unsigned int)buf[1];
}

unsigned short getBlockNo(char *buf) {
  vprintf("Inside getBlockNo:\n");
  vprintf("(unsigned char) buf[2] = 0x%x\n", (unsigned)(unsigned char)buf[2] );
  vprintf("(unsigned char) buf[3] = 0x%x\n", (unsigned)(unsigned char)buf[3] );

  // vprintf("(unsigned) buf[2] = 0x%x\n", (unsigned)buf[2] );
  // vprintf("(unsigned) buf[3] = 0x%x\n", (unsigned)buf[3] );

  // vprintf("MOST_SIG = %d\n", (unsigned)buf[2] << 8 );
  // vprintf("LEAST_SIG = %d\n", (unsigned)buf[3] );

  return ((unsigned short)(unsigned char)buf[2] << 8) + ((unsigned short)(unsigned char)buf[3]);
}

// Could probably collapse pack_rrq and pack_wrq into one
// just pass in the type (either WRQ or RRQ)

int pack_rrq(char *buf, char *fileName, const char *mode) {
  int length = 2 + strlen(fileName) + strlen(mode) + 2;
  if(length > MAXBUFLEN){
    return -1;
  }
  buf[0] = MOST_SIG(RRQ);
  buf[1] = LEAST_SIG(RRQ);
  memcpy(buf + 2, fileName, strlen(fileName) + 1);
  memcpy(buf + strlen(fileName) + 3, mode, strlen(mode) + 1);
  return length;
}

int pack_wrq(char *buf, char *fileName, const char *mode) {
  int length = 2 + strlen(fileName) + strlen(mode) + 2;
  if(length > MAXBUFLEN){
    return -1;
  }
  buf[0] = MOST_SIG(WRQ);
  buf[1] = LEAST_SIG(WRQ);
  memcpy(buf + 2, fileName, strlen(fileName) + 1);
  memcpy(buf + strlen(fileName) + 3, mode, strlen(mode) + 1);
  return length;
}

int pack_data(char *buf, short block, char *data, int dataLength) {
  int length = 2 + 2 + dataLength;
  if(length > MAXBUFLEN){
    return -1;
  }

  vprintf("Inside pack_data: \n");
  buf[0] = MOST_SIG(DATA);
  buf[1] = LEAST_SIG(DATA);
  buf[2] = MOST_SIG(block);
  vprintf("buf[2] = 0x%x\n", (unsigned)(unsigned char)buf[2] );
  buf[3] = LEAST_SIG(block);
  vprintf("buf[3] = 0x%x\n", (unsigned)(unsigned char)buf[3] );

  memcpy(buf + 4, data, dataLength);
  return length;
}

int pack_ack(char *buf, short block) {
  buf[0] = MOST_SIG(ACK);
  buf[1] = LEAST_SIG(ACK);
  buf[2] = MOST_SIG(block);
  buf[3] = LEAST_SIG(block);
  return 4;
}

int pack_error(char *buf, error_code ec, char *errMsg) {
  int length = 2 + 2 + strlen(errMsg) + 1;
  if(length > MAXBUFLEN){
    return -1;
  }
  buf[0] = MOST_SIG(ERROR);
  buf[1] = LEAST_SIG(ERROR);
  buf[2] = MOST_SIG(ec);
  buf[3] = LEAST_SIG(ec);
  memcpy(buf + 4, errMsg, strlen(errMsg) + 1);
  return length;
}

int send_packet(int sockfd, send_req request, tftp_state *state) {
  int bytes_sent;
  if(request.op == 0){
    return -1;
  }
  vprintf("request.TID = %d\n", request.TID);
  vprintf("state->TID = %d\n", state->TID);

  if(request.TID == state->TID){
    state->wait_time = time(NULL);
  }
  if((bytes_sent = sendto(sockfd, request.buf, request.length, 0, 
      (struct sockaddr *) & request.address, sizeof(struct sockaddr))) == -1) {
    perror("send_packet->sendto");
    exit(EXIT_FAILURE);
  }

  vprintf("sent %d bytes to %s %d\n", bytes_sent, inet_ntoa(((struct sockaddr_in *)&request.address)->sin_addr), ntohs(((struct sockaddr_in *)&request.address)->sin_port));

  return bytes_sent;
}

int recvfrom_timeout(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen, tftp_state state){
  int code,timedout = 0;
  do{
    code = recvfrom(sockfd,buf,len,0,from,fromlen);
    usleep(1000 * 1);
    timedout = state.wait_time + TIMEOUT < time(NULL); 
  }while(code <= 0 && !timedout);
  if(code > 0){
    vprintf("got a packet!\n");
    return code;
  }else{
    vprintf("response timed out! (since: %zu, now: %zu)\n",state.wait_time,time(NULL));
    return -1;
  }
  
}
