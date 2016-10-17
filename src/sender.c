#include "packet_implem.h"
#include "fonctions_communes.h"
#include "connection_and_transfer.h"
#include "selective_repeat_sender.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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

  int sfd = create_socket(NULL,&addr); // file descriptor de la connexion

  if(sfd == -1){
    fprintf(stderr, "Not able to create socket : sfd = -1\n");
  }
  int fd; // file descriptor sur lequel lire

  if(file_name != NULL){
    fd = open(file_name,O_RDONLY); // file descriptor du fichier à lire
  }
  else{
    fd = STDIN_FILENO;
  }

  if(fd == -1){
    fprintf(stderr, "Not able to open file %s\n",file_name);
  }

  return 0;
}
