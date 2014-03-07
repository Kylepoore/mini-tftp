/*
** fsm_client.c
**
** Authors: Kyle Poore, Robert Correiro
**
**
*/
#include "tftp.h"
#include "packet.h"

tftp_state setup_client(protocol_state st) {
  tftp_state state = {.state = st, .block = 0};
  return state;
}

int file_writer(send_req *request, tftp_state *client, char *buf, unsigned int bytes) {
  int length = 0;
  size_t bytes_written = 0;

  if ((bytes_written = fwrite(buf + 4, sizeof(char), 
                              bytes - 4, client->fp)) < bytes - 4) {
    vprintf("Only wrote %zu of %d bytes from buffer.\n", bytes_written ,bytes);
    length = pack_error(request->buf, DISK_FULL, "Error: Disk is full.");
    request->op = ERROR;    
  }
  vprintf("Wrote %d bytes from buffer.\n", bytes);
  return length;
}

int build_req(send_req *request, tftp_state *client, 
  struct sockaddr address, char *fn, char *buf, unsigned int bytes) {

  int op = getOpCode(buf);
  int block = getBlockNo(buf);
  int length = 0;

  request->op = 0;

  switch(client->state) {
    case INIT_READER:
      vprintf("client->state: INIT_READER\n");
      switch(op) {
        case DATA:
          vprintf("Packet Type: DATA\n");
          if (block != 1) {
            return -1;
          }

          FILE *fp = NULL;
          if ((fp = fopen(fn, "w")) == NULL) {
            perror("start_reader: fopen");
            exit(EXIT_FAILURE);  
          }

          client->fp = fp;
          length = file_writer(request, client, buf, bytes);
          client->state = READER;
          break;

        case ERROR:
          vprintf("Packet Type: ERROR\n");
          fclose(client->fp);
          return -1;
          break;

        default:
          vprintf("Unknown Packet Type received.\n");
          return -1;
          break;
      }
      break;

    case READER:
      vprintf("client->state: READER\n");
      switch(op) {
        case DATA:
          if (block != client->block + 1) {
            vprintf("Error with block number. Got: %d\n", block);
            return 0;
          }
          vprintf("Block %d received.\n", block);

          length = file_writer(request, client, buf, bytes);

          vprintf("Preparing to send ACK for block %d.\n", block);
          length = pack_ack(request->buf, block);
          request->op = ACK;
          break;

        case ERROR:
          vprintf("Packet Type: ERROR\n");
          fclose(client->fp);
          return -1;
          break;

        default:
          break;
      }
      break;

    // case WRITER:
    //   switch(op) {

    //   }
    //   break;

    default:
      break;
  }

  request->address = address;
  request->length = length;

  return 1;
}