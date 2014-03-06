#include "tftp.h"
#include "packet.h"

int getOpCode(char *buf) {
  return ((int)buf[0] << 8) + buf[1];
}

int pack_rrq(char *buf, char *fileName, char *mode) {
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

int pack_wrq(char *buf, char *fileName, char *mode) {
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
  buf[0] = MOST_SIG(DATA);
  buf[1] = LEAST_SIG(DATA);
  buf[2] = MOST_SIG(block);
  buf[3] = LEAST_SIG(block);
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

int send_packet(int sockfd, send_req request, int verbose) {
  int bytes_sent;
  if((bytes_sent = sendto(sockfd, request.buf, request.length, 0, 
      (struct sockaddr *) & request.address, sizeof(struct sockaddr))) == -1) {
    perror("send");
    exit(EXIT_FAILURE);
  }

  vprintf("packet sent, %d bytes\n", bytes_sent);
} 
