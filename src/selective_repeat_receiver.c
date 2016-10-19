#include "selective_repeat_receiver.h"

/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 */
int selective_repeat_receive(int sfd,FILE * f){

  uint8_t window = 5; // window [0,31]
  pkt_t * receiving_buffer[window]; // buffer contenant les segment à envoyer

  uint16_t seq_num_expected = 0;
  size_t n = -1; // nombre de bytes lus
  char buf_packet[524]; // packet
  char buf_ack[12]; // acknowledgement
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
      if(FD_ISSET(sfd,&read_fd) && FD_ISSET(sfd,&write_fd)){
        memset(buf_packet,0,524);
        n = recv(sfd,buf_packet,524,0); // lecture d'un packet de max 524 bytes
        printf("ack\n");
        // creation d'une structure et placement dans le buffer
        if(n > 0){

          pkt_t * new_pkt = pkt_new();
          pkt_t * ack_pkt = pkt_new();
          pkt_status_code status_decode = pkt_decode(buf_packet, n, new_pkt);

          if(status_decode != PKT_OK){
            fprintf(stderr, "Not able to decode\n");

            // send ack
          }
          else{

            write(fd,buf_packet,524);
            // ENVOYER ack
            uint8_t num_ack;
            // Création de l'ack
            if(pkt_get_seqnum(new_pkt) == seq_num_expected) // si packet attendu...
            {
              num_ack = seq_num_expected;
              // find new expected seqnum
              // send_ack and write all in sequence packet from buffer to fd
            }
            else if(((pkt_get_seqnum(new_pkt) > seq_num_expected) && ((pkt_get_seqnum(new_pkt) - seq_num_expected) <= window)) || ((pkt_get_seqnum(new_pkt) < seq_num_expected) && ((pkt_get_seqnum(new_pkt) + 255 - seq_num_expected) <= window))) // si packet dans le désordre
            {
              num_ack = seq_num_expected;

              // place dans le buffer
              uint8_t position_allowed_in_buffer = 0;
              while(position_allowed_in_buffer < window && receiving_buffer[position_allowed_in_buffer] != NULL) {
                position_allowed_in_buffer++;
              }
              receiving_buffer[position_allowed_in_buffer] = new_pkt;

            }
            else // si duplicata
            {
              num_ack = seq_num_expected; // Discard and send ack
            }

            pkt_set_length(ack_pkt,0);
            pkt_set_type(ack_pkt,PTYPE_ACK);
            pkt_set_window(ack_pkt,window);
            pkt_set_seqnum(ack_pkt,num_ack); 
            memset(buf_ack,0,12);
            size_t len = 12;
            pkt_status_code status_code = pkt_encode(ack_pkt,buf_ack,&len);

            if(status_code != PKT_OK){
              fprintf(stderr, "Not able to encode the structure\n");
            }
            else{
              err_num = send(sfd,buf_ack,sizeof(buf_ack),0);

              if(err_num == -1){
                fprintf(stderr, "send error %s\n",strerror(errno));

              }
            }

          }

        }

      }

    }

  }
  return 0;
}
