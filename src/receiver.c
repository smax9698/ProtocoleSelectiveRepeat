#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fonctions_communes.h"
#include "connection_and_transfer.h"
#include "selective_repeat_receiver.h"

int main(int argc,char *argv[]){

  // lecture et stockage des arguments

  char * file_name = NULL;
  char * host_name = NULL;
  int err_num = 0;

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
    return -1;
  }

  // establish connection

  int sfd = create_socket(&addr,NULL); // file descriptor de la connexion

  if(sfd == -1){
    fprintf(stderr, "Not able to create socket : sfd = -1\n");
  }

  if(sfd > 0 && wait_for_client(sfd) < 0){ /* Connected */
    fprintf(stderr,"Could not connect the socket after the first message.\n");
    close(sfd);
    return -1;
  }

  int fd; // file descriptor sur lequel ecrire

  if(file_name != NULL){
    fd = open(file_name,O_WRONLY); // file descriptor du fichier sur lequel ecrire
  }
  else{
    fd = STDOUT_FILENO;
  }

  err_num = selective_repeat_receive(sfd,fd);
  if(err_num != 0){
    fprintf(stderr, "erreur dans selective_repeat_receive\n");
  }
  //printf("end of receiver\n");
  return 0;
}
