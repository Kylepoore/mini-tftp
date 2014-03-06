#ifndef PACKET_H
#define PACKET_H

#define MOST_SIG(a) ((short)(a) >> 8)
#define LEAST_SIG(a) (((short)(a) << 8) >> 8)

extern const char *mode_netascii;

int getOpCode(char *buf);

int pack_rrq(char *buf, char *fileName, const char *mode);

int pack_wrq(char *buf, char *fileName, const char *mode);

int pack_data(char *buf, short block, char *data, int dataLength);

int pack_ack(char *buf, short block);

int pack_error(char *buf, error_code ec, char *errMsg);

int send_packet(int sockfd, send_req request);

void free_send_req(send_req request);

#endif
