#include "selective_repeat_sender.h"

/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 */
int selective_repeat_receive(int sfd,FILE * f){

  pkt_t * sending_buffer[32]; // buffer contenant les segment à envoyer
  uint8_t window = 1; // window [0,31]

  size_t n = -1; // nombre de bytes lus
  char buf[524]; // payload size + header and CRC sizes
  int fd = fileno(f);
  int max_fd = sfd+1;
  int err_num;
  fd_set read_fd, write_fd;

  while(n != 0){

    //memset((void *)buf,0,10);
    //n =  read(fd, (void *) buf, 10);


    FD_ZERO(&read_fd);
    FD_ZERO(&write_fd);

    FD_SET(sfd,&read_fd);
    FD_SET(sfd,&write_fd);

    err_num = select(max_fd, &read_fd, &write_fd, NULL,NULL);

    if(err_num == -1){
      fprintf(stderr, "%s\n",strerror(errno));
    }
    else{

      // réception des acknowledgments
      if(FD_ISSET(sfd,&read_fd)){

      }

      // envoi des packets
      if(FD_ISSET(sfd,&write_fd)){

      }

    }

  }
  return 0;
}
