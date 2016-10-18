#include "selective_repeat_sender.h"

/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 */
int selective_repeat_send(int sfd,FILE * f){

  pkt_t * sending_buffer[31]; // buffer contenant les segment à envoyer
  uint8_t window = 5; // window [0,31]
  uint16_t lastack = 0; // dernier numero de sequence acknowledged
  uint16_t seq_num = 0; // numero de sequence actuel

  uint8_t authorized_buff_max = 1; // taille de la window du receiver
  ssize_t err;
  size_t n = -1; // nombre de bytes lus
  char buf_payload[512]; // payload
  char buf_packet[524]; // packet

  int fd = fileno(f);
  int max_fd = sfd+1;

  fd_set read_fd, write_fd;

  while(n != 0){

    //memset((void *)buf,0,10);
    //n =  read(fd, (void *) buf, 10);

    FD_ZERO(&read_fd);
    FD_ZERO(&write_fd);

    FD_SET(sfd,&read_fd);
    FD_SET(sfd,&write_fd);

    err = select(max_fd, &read_fd, &write_fd, NULL,NULL);

    if(err == -1){
      fprintf(stderr, "%s\n",strerror(errno));
    }
    else{

      // réception des acknowledgments
      if(FD_ISSET(sfd,&read_fd)){

      }

      // envoi des packets
      if(FD_ISSET(sfd,&write_fd)){

          // tant qu'il y a de la place dans le buffer, envoyer des packets.
          while(window > 0 && n != 0){

            n = read(fd,buf_payload,512); // lecture de max 512 bytes pour mettre dans le payload
            printf("%d\n",n);
            pkt_t * new_pkt = pkt_new();

            pkt_set_length(new_pkt,n);
            pkt_set_type(new_pkt,PTYPE_DATA);

            seq_num = (seq_num+1)%256;

            pkt_set_seqnum(new_pkt,seq_num);
            pkt_set_window(new_pkt,window);
            pkt_set_timestamp(new_pkt,1);
            pkt_set_payload(new_pkt,buf_payload,n);

            pkt_status_code status_code;
            size_t len = 524;
            status_code = pkt_encode(new_pkt,buf_packet,&len);
            if(status_code != PKT_OK){
              fprintf(stderr, "Not able to encode the structure\n");
              return -1;
            }

            uint8_t position_allowed_in_buffer = 0;
            while(position_allowed_in_buffer < 31 && sending_buffer[position_allowed_in_buffer] != NULL) {
              position_allowed_in_buffer++;
            }

            sending_buffer[position_allowed_in_buffer] = new_pkt;

            err = send(sfd,buf_packet,sizeof(buf_packet),0);
            printf("packet sended\n");
            if(err == -1){
              fprintf(stderr, "send error %s\n",strerror(err));

            }

            window--;
          }

      }

    }
    printf("%d\n",n);
  }
  return 0;
}
