#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include "fonctions_communes.h"
#include "connection_and_transfer.h"

int main(int argc,char *argv[]){

  // lecture et stockage des arguments

  char * file_name = NULL;
  char * host_name = NULL;

  uint16_t port;
  if(read_entries(argc,argv,&file_name,&host_name,&port) == -1){
    fprintf(stderr, "Error in entries\n");
    return -1;
  }

  // resolve the host_name

  struct sockaddr_in6 addr;

  const char *err = real_address(host_name,port,&addr);

  if (err) {
    fprintf(stderr, "Could not resolve hostname %s: %s\n", host_name, err);
    return EXIT_FAILURE;
  }

  // establish connection

  int sfd = create_socket(NULL,&addr); /* connected */

  
  return 0;
}
