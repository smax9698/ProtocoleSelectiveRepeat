#include "selective_repeat_receiver.h"

/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 */
int selective_repeat_receive(int sfd,FILE * f){

  pkt_t * receiving_buffer[31]; // buffer contenant les segment à envoyer
  uint8_t window = 1; // window [0,31]

  size_t n = -1; // nombre de bytes lus
  char buf_packet[524]; // packet
  int fd = fileno(f);
  int max_fd = sfd+1;
  int err_num;
  fd_set read_fd, write_fd;

  while(n != 0){

    FD_ZERO(&read_fd);
    FD_ZERO(&write_fd);

    FD_SET(sfd,&read_fd);
    FD_SET(sfd,&write_fd);

    err_num = select(max_fd, &read_fd, &write_fd, NULL,NULL);

    if(err_num == -1){
      fprintf(stderr, "%s\n",strerror(errno));
    }
    else{

      // réception des packets
      if(FD_ISSET(sfd,&read_fd)){
        memset(buf_packet,0,524);
        n = recv(sfd,buf_packet,524,0); // lecture d'un packet de max 524 bytes
        if(n > 0){
          printf("we write\n");
          write(fd,buf_packet,n);
        }
      }

      // envoi des acknowledgments
      if(FD_ISSET(sfd,&write_fd)){

      }

    }

  }
  return 0;
}
