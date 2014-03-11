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

int done(unsigned int bytes) {
  return (bytes - 4) < 512;
}

int error_handler(tftp_state *client, char *buffer, int error_code) {
  vprintf("Packet Type: ERROR\n");
  switch(error_code){
    case UNDEFINED :
      vprintf("undefined error occurred: %s\n",buffer);
      exit(EXIT_FAILURE);
      break; 
    case FILE_NOT_FOUND :
      vprintf("file not found: %s\n",buffer);
      exit(EXIT_FAILURE);
      break; 
    case ACCESS_VIOLATION :
      vprintf("permision denied: %s\n",buffer);
      exit(EXIT_FAILURE);
      break; 
    case DISK_FULL :
      vprintf("disk full: %s\n",buffer);
      exit(EXIT_FAILURE);
      break; 
    case ILLEGAL_OP :
      vprintf("illecal op code: %s\n",buffer);
      exit(EXIT_FAILURE);
      break; 
    case UNKNOWN_TID :
      vprintf("unknown tid: %s\n",buffer);
      break; 
    case FILE_EXISTS :
      vprintf( "file already exists: %s\n",buffer);
      exit(EXIT_FAILURE);
      break; 
    case NO_SUCH_USER :
      vprintf("no such user: %s",buffer);
      exit(EXIT_FAILURE);
      break;
    default:
      vprintf("unknown error: %s",buffer);
  } 
  client->done = 1;
  return -1;
}

void data_handler(send_req *request, tftp_state *client, 
  char *buf, unsigned int bytes, unsigned short block) {
  
  size_t bytes_written = 0;

  vprintf("Writing %d bytes from buffer.\n", bytes - 4);
  if ((bytes_written = fwrite(buf + 4, sizeof(char), bytes - 4, client->fp)) < bytes - 4 
       && !client->done) {
    vprintf("Wrote %zu of %d bytes from buffer.\n", bytes_written, bytes);
    vprintf("Preparing to send ERROR for block %d.\n", block);
    request->length = pack_error(request->buf, DISK_FULL, "Error: Disk is full.");
    request->op = ERROR;    
    client->done = 1;
    return;
  }

  // If this is the first and only, or last DATA packet.
  if (done(bytes)) {
    client->done = 1;
  }

  vprintf("Sending ACK for block %d.\n", block);
  request->length = pack_ack(request->buf, client->block++);
  request->op = ACK;
}

int build_req(send_req *request, tftp_state *client, 
  struct sockaddr address, char *fn, char *buf, unsigned int bytes) {

  int op = getOpCode(buf);
  unsigned short block = getBlockNo(buf);
  int length = 0;
  char data_buf[512];
  size_t bytes_read = 0;

  int port = ntohs(((struct sockaddr_in *)&address)->sin_port);

  request->op = 0;

  switch(client->state) {

    case INIT:
      vprintf("client->state: INIT\n");
        switch(op) {
          case DATA:
            vprintf("Type: DATA | Block: %d\n", block);
          
            // First data block # should be 1. Otherwise, ignore it. 
            if (block != 1) {
              return 0;
            }
            client->wait_time = time(NULL);
            client->TID = port;

            if ((client->fp = fopen(fn, "w")) == NULL) {
              perror("INIT->DATA: fopen");
              exit(EXIT_FAILURE);  
            }

            client->state = READER;
            data_handler(request, client, buf, bytes, block);
            break;

          case ACK:
            vprintf("Type: ACK  |  Block: %d\n", block);

            // First ACK block # should be 0. Otherwise, ignore it. 
            if (block != 0) {
              return 0;
            }

            client->wait_time = time(NULL);
            client->TID = port;
            
            if ((client->fp = fopen(fn, "r")) == NULL) {
              perror("INIT->ACK: fopen");
              exit(EXIT_FAILURE);  
            }


            bytes_read = fread(data_buf, sizeof(char), 512, client->fp);
            vprintf("Read %zu bytes from file.\n", bytes_read);


            // REFACTOR: same as WRITER
            vprintf("Sending DATA block %d.\n", client->block);
            request->length = pack_data(request->buf, client->block++, data_buf, bytes_read);
            request->op = DATA;

            // Still need to wait for final ACK, before terminating.
            if (bytes_read < 512) {
              client->state = FINAL;
            } else {
              client->state = WRITER;
            }
            break;

          case ERROR:
            return error_handler(client,buf,getErrorNo(buf));

          default:  
            return 0;  // Ignore packet
        }
      break;

    case READER:
      vprintf("client->state: READER\n");

      switch(op) {
        case DATA:
          vprintf("Type: DATA | Block: %d\n", block);
          if (block != client->block) {
            vprintf("Error with block number. Got: %d\n", block);
            return 0;
          }

          client->wait_time = time(NULL);
          data_handler(request, client, buf, bytes, block);
          break;

        case ERROR:
          return error_handler(client, buf, getErrorNo(buf));

        default:  
          return 0;  // Ignore packet
      }
      break;

    case WRITER:
      vprintf("client->state: WRITER\n");
      switch(op) {
        case ACK:
          vprintf("Type: ACK  |  Block: %d\n", block);
          vprintf("client->block = %d\n", client->block);
          vprintf("block + 1 = %d\n", block + 1);          

          vprintf("sizeof(client->block) = %zu\n", sizeof(client->block));
          vprintf("sizeof(block) = %zu\n", sizeof(block));

          // ACK block # should be equal to my previous. If not, ignore it. 
          if ((unsigned short)(block + 1) != client->block) {
            vprintf("FAILED: block + 1 = %d, client->block = %d\n", block + 1, client->block);
            return 0;
          }

          client->wait_time = time(NULL);
          bytes_read = fread(data_buf, sizeof(char), 512, client->fp);
          vprintf("Read %zu bytes from file.\n", bytes_read);


          // REFACTOR: same as INIT_WRITER
          vprintf("Sending DATA block %d.\n", client->block);
          request->length = pack_data(request->buf, client->block++, data_buf, bytes_read);
          request->op = DATA;

          vprintf("client->block = %d\n", client->block);
          // Still need to wait for final ACK, before terminating.
          if (bytes_read < 512) {
            client->state = FINAL;
          }
          break;

        case ERROR:
          return error_handler(client,buf,getErrorNo(buf));

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

          client->wait_time = time(NULL);
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
  request->TID = port;

  return 1;
}
