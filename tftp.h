#ifndef TFTP_H
#define TFTP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define vprintf(format, ...) do {                  \
  if (verbose)                                     \
    fprintf(stderr, format, ##__VA_ARGS__);        \
} while(0)

#define MAXBUFLEN 1024

typedef enum {
  UNDEFINED = 0,
  FILE_NOT_FOUND = 1,
  ACCESS_VIOLATION = 2,
  DISK_FULL = 3,
  ILLEGAL_OP = 4,
  UNKNOWN_TID = 5,
  FILE_EXISTS = 6,
  NO_SUCH_USER = 7
} error_code;

typedef enum {
  RRQ = 1,
  WRQ = 2,
  DATA = 3,
  ACK = 4,
  ERROR = 5
} op_code;

typedef enum {
  WAITING,
  SENDING,
  RECEIVING,
  SHUTDOWN,
  INIT_READER,
  READER,
  INIT_WRITER,
  WRITER
} protocol_state;

typedef struct {
  protocol_state state;
  int block;
  FILE *fp;
  int done;
} tftp_state;

typedef struct {
  struct sockaddr address;
  char buf[MAXBUFLEN];
  int length;
  op_code op;
} send_req;

extern int verbose;

#endif
