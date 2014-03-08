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
  tftp_state state = {.state = st, .block = 1};
  return state;
}

// size_t file_reader(FILE *fp, char *buf, unsigned int bytes) {
//   vprintf("Reading %d bytes from file.\n", bytes);
//   return fread(buf + 4, sizeof(char), bytes - 4, fp);
// }

// size_t file_writer(FILE *fp, char *buf, unsigned int bytes) {
//   vprintf("Writing %d bytes from buffer.\n", bytes);
//   return fwrite(buf + 4, sizeof(char), bytes - 4, fp);
// }

int error_handler(tftp_state *client) {
  vprintf("Packet Type: ERROR\n");
  client->done = 1;
  return -1;
}

int build_req(send_req *request, tftp_state *client, 
  struct sockaddr address, char *fn, char *buf, unsigned int bytes) {

  int op = getOpCode(buf);
  int block = getBlockNo(buf);
  int length = 0;
  char data_buf[512];
  size_t bytes_written = 0;
  size_t bytes_read = 0;

  request->op = 0;

  switch(client->state) {

    case INIT:
      vprintf("client->state: INIT\n");
        switch(op) {
          case DATA:
            vprintf("Packet Type: DATA\n");
          
            // First data block # should be 1. Otherwise, ignore it. 
            if (block != 1) {
              return 0;
            }

            if ((client->fp = fopen(fn, "w")) == NULL) {
              perror("INIT->DATA: fopen");
              exit(EXIT_FAILURE);  
            }

            // If requested file fits entirely in first DATA packet.
            if (bytes - 4 < 512) {
              client->done = 1;
            }

            // REFACTOR: Same as READER
            vprintf("Writing %d bytes from buffer.\n", bytes - 4);
            if ((bytes_written = fwrite(buf + 4, sizeof(char), bytes - 4, client->fp)) < bytes - 4 
                 && !client->done) {
              vprintf("Wrote %zu of %d bytes from buffer.\n", bytes_written, bytes);
              vprintf("Preparing to send ERROR for block %d.\n", block);
              length = pack_error(request->buf, DISK_FULL, "Error: Disk is full.");
              request->op = ERROR;    
              client->done = 1;
              break;
            }

            // REFACTOR: Same as READER
            vprintf("Sending ACK for block %d.\n", block);
            length = pack_ack(request->buf, client->block++);
            request->op = ACK;

            client->state = READER;
            break;

          case ACK:
            vprintf("Packet Type: ACK\n");

            // First ACK block # should be 0. Otherwise, ignore it. 
            if (block != 0) {
              return 0;
            }

            if ((client->fp = fopen(fn, "r")) == NULL) {
              perror("INIT->ACK: fopen");
              exit(EXIT_FAILURE);  
            }

            bytes_read = fread(data_buf, sizeof(char), 512, client->fp);
            vprintf("Read %zu bytes from file.\n", bytes_read);


            // REFACTOR: same as WRITER
            vprintf("Sending DATA block %d.\n", client->block);
            length = pack_data(request->buf, client->block++, data_buf, bytes_read);
            request->op = DATA;

            // Still need to wait for final ACK, before terminating.
            if (bytes_read < 512) {
              client->state = FINAL;
            } else {
              client->state = WRITER;
            }
            break;

          case ERROR:
            return error_handler(client);

          default:  
            return 0;  // Ignore packet
        }
      break;

    case READER:
      vprintf("client->state: READER\n");

      switch(op) {
        case DATA:
          if (block != client->block) {
            vprintf("Error with block number. Got: %d\n", block);
            return 0;
          }
          vprintf("Block %d received.\n", block);

          // REFACTOR: Same as READER
          vprintf("Writing %d bytes from buffer.\n", bytes - 4);
          if ((bytes_written = fwrite(buf + 4, sizeof(char), bytes - 4, client->fp)) < bytes - 4 
               && !client->done) {
            vprintf("Wrote %zu of %d bytes from buffer.\n", bytes_written, bytes);
            vprintf("Preparing to send ERROR for block %d.\n", block);
            length = pack_error(request->buf, DISK_FULL, "Error: Disk is full.");
            request->op = ERROR;    
            client->done = 1;
            break;
          }

          // If this is last DATA packet.
          if (bytes - 4 < 512) {
            client->done = 1;
          }

          // REFACTOR: Same as INIT
          vprintf("Sending ACK for block %d.\n", block);
          length = pack_ack(request->buf, client->block++);
          request->op = ACK;
          break;

        case ERROR:
          return error_handler(client);

        default:  
          return 0;  // Ignore packet
      }
      break;

    case WRITER:
      vprintf("client->state: WRITER\n");
      switch(op) {
        case ACK:
          vprintf("Packet Type: ACK\n");

          // ACK block # should be equal to my previous. If not, ignore it. 
          if (block != client->block - 1) {
            return 0;
          }

          bytes_read = fread(data_buf, sizeof(char), 512, client->fp);
          vprintf("Read %zu bytes from file.\n", bytes_read);


          // REFACTOR: same as INIT_WRITER
          vprintf("Sending DATA block %d.\n", client->block);
          length = pack_data(request->buf, client->block++, data_buf, bytes_read);
          request->op = DATA;

          // Still need to wait for final ACK, before terminating.
          if (bytes_read < 512) {
            client->state = FINAL;
          }
          break;

        case ERROR:
          return error_handler(client);

        default:  
          return 0;  // Ignore packet
      }
      break;

    case FINAL:
      vprintf("client->state: FINAL\n");

      switch(op) {
        case ACK:
          vprintf("Packet Type: ACK\n");

          // ACK block # should be equal to my previous. If not, ignore it. 
          if (block != client->block - 1) {
            return 0;
          }

          // Received final ACK, success.
          client->done = 1;
          return 2;

        default:  
          return 0;  // Ignore packet
      }

    default:
      vprintf("fsm_client: Invalid client state.");
      exit(EXIT_FAILURE);
  }

  request->address = address;
  request->length = length;

  return 1;
}