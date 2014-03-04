/*
** tftp-server.c
** by Kyle Poore.
** Created: Mar 3, 2014
*/
#include "tftp.h"
#include "server.h"
#include "client.h"


int main(int argc,char **argv) {
  char *port = "3335";
  char c;
  int verbose = 0;
  char clientMode = 0;
  int isServer = 0;
  char *filename;
  char *host;
  while((c = getopt(argc,argv,"lvrwp:")) != -1){
    switch(c){
      case 'l':
        //servermode
        isServer = 1;
        break;
      case 'v':
        //print all debug info
        verbose = 1;
        break;
      case 'r':
        //copy file from host to local
        clientMode = 'r';
        break;
      case 'w':
        //copy file from local to host
        clientMode = 'w';
        break;
      case 'p':
        //specify port number
        port = optarg;
        break;
      default:
        printf("invalid option\n"); 
    }
  }
  if((clientMode && isServer) || (!isServer && !(clientMode == 'r' || clientMode == 'w'))){
    printf("argument parsing failed\n");
    exit(EXIT_FAILURE);
  }

  if(!isServer && optind > argc){
    printf("host name or file name not specified\n");
    exit(EXIT_FAILURE);
  }

  if(!isServer){
    host = argv[optind++];
    filename = argv[optind];
  }

  if(isServer){
    startServer(port,verbose);
  }else{
    startClient(port, filename, host, clientMode, verbose);
  }
	return EXIT_SUCCESS;
}
