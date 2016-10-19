#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include "fonctions_communes.h"
#include "connection_and_transfer.h"
#include "selective_repeat_receiver.h"

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

  if(err){
    fprintf(stderr,"Could not resolve hostname %s: %s\n",host_name,err);
    return EXIT_FAILURE;
  }

  // establish connection

  int sfd = create_socket(&addr,NULL); // file descriptor de la connexion

  if(sfd == -1){
    fprintf(stderr, "Not able to create socket : sfd = -1\n");
  }

  if(sfd > 0 && wait_for_client(sfd) < 0){ /* Connected */
    fprintf(stderr,"Could not connect the socket after the first message.\n");
    close(sfd);
    return EXIT_FAILURE;
  }

  FILE * f; // file descriptor sur lequel ecrire

  if(file_name != NULL){
    f = fopen(file_name,"w"); // file descriptor du fichier sur lequel ecrire
  }
  else{
    f = stdout;
  }

  selective_repeat_receive(sfd,f);
  return 0;
}
