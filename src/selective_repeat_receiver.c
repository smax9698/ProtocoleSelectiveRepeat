#include "selective_repeat_receiver.h"


int send_ack(int sfd, uint8_t window, uint16_t seq_num){

  int err_num = 0;
  pkt_t * ack_pkt = pkt_new();
  char buf_ack[12]; // acknowledgement
  pkt_set_length(ack_pkt,0);
  pkt_set_type(ack_pkt,PTYPE_ACK);
  pkt_set_window(ack_pkt,window);
  pkt_set_seqnum(ack_pkt,seq_num);
  memset(buf_ack,0,12);
  size_t len = 12;
  pkt_status_code status_code = pkt_encode(ack_pkt,buf_ack,&len);

  if(status_code != PKT_OK){
    fprintf(stderr, "Not able to encode the structure\n");
    return -1;
  }
  else{
    err_num = send(sfd,buf_ack,sizeof(buf_ack),0);

    if(err_num == -1){
      fprintf(stderr, "send error %s\n",strerror(errno));
      return -1;
    }
  }
  pkt_del(ack_pkt);
  return 0;
}
/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 */
int selective_repeat_receive(int sfd,FILE * f){

  uint8_t window = 31; // window [0,31]
  pkt_t * receiving_buffer[window]; // buffer contenant les segment à envoyer
  for (size_t i = 0; i < 31; i++) {
    receiving_buffer[i] = NULL;
  }

  uint16_t seq_num_expected = 0;
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
      if(FD_ISSET(sfd,&read_fd) && FD_ISSET(sfd,&write_fd)){
        memset(buf_packet,0,524);
        n = recv(sfd,buf_packet,524,0); // lecture d'un packet de max 524 bytes
        fprintf(stderr,"reception d'un packet de taille : %d\n",n);

        // creation d'une structure et placement dans le buffer
        if(n > 0){

          pkt_t * new_pkt = pkt_new();

          // decodage buf_packet -> new_pkt
          pkt_status_code status_decode = pkt_decode(buf_packet, n, new_pkt);

          if(status_decode != PKT_OK){
            fprintf(stderr, "Not able to decode\n");

            if(status_decode == E_CRC){
              send_ack(sfd,window,seq_num_expected);
            }

          }
          else{

            if(pkt_get_seqnum(new_pkt) == seq_num_expected) // si packet attendu...
            {
              fprintf(stderr,"packet reçu IN ORDER num_seq : %d\n",pkt_get_seqnum(new_pkt));

              // WRITE PACKET payload
              write(fd,pkt_get_payload(new_pkt),pkt_get_length(new_pkt));
              fprintf(stderr,"write packet seqnum : %d\n",pkt_get_seqnum(new_pkt));
              // SEND ack

              seq_num_expected = (seq_num_expected+1)%256;

              //if(seq_num_expected < 20 || seq_num_expected > 24){ //SIMULE PERTE D'ACK
              //send_ack(sfd,window,seq_num_expected);
              //fprintf(stderr,"send ack for seqnum : %d with seq_num_expected : %d\n",pkt_get_seqnum(new_pkt),seq_num_expected);
              //}
              bool still_in_sequence_packet = true;
              while (still_in_sequence_packet) {
                still_in_sequence_packet = false;
                size_t i;
                for (i = 0; i < 31 && !still_in_sequence_packet; i++) {

                  if(receiving_buffer[i] != NULL){
                    if(pkt_get_seqnum(receiving_buffer[i]) == seq_num_expected){
                      // WRITE packet
                      write(fd,pkt_get_payload(receiving_buffer[i]),pkt_get_length(receiving_buffer[i]));
                      fprintf(stderr,"write packet seqnum : %d\n",pkt_get_seqnum(new_pkt));
                      seq_num_expected = (seq_num_expected+1)%256;

                      // send ack

                      //send_ack(sfd,window,seq_num_expected);
                      //fprintf(stderr,"send ack for seqnum : %d with seq_num_expected : %d\n",pkt_get_seqnum(receiving_buffer[i]),seq_num_expected);

                      // RETIRE DU buffer
                      receiving_buffer[i] = NULL;
                      still_in_sequence_packet = true;
                    }
                  }
                }
              }
              send_ack(sfd,window,seq_num_expected);
<<<<<<< HEAD

=======
>>>>>>> 7931adbfd3c5b43132e3161c8011a5bc3617f79c
            }
            else if(((pkt_get_seqnum(new_pkt) > seq_num_expected) && ((pkt_get_seqnum(new_pkt) - seq_num_expected) <= window)) || ((pkt_get_seqnum(new_pkt) < seq_num_expected) && ((pkt_get_seqnum(new_pkt) + 255 - seq_num_expected) <= window))) // si packet dans le désordre
            {
              fprintf(stderr,"packet reçu OUT OF ORDER : num_seq %d\n",pkt_get_seqnum(new_pkt));
              // VERIFIER PAQUET PAS ENCORE DANS BUFFER
              // place dans le buffer
              uint8_t position_allowed_in_buffer = 0;
              while(position_allowed_in_buffer < window && receiving_buffer[position_allowed_in_buffer] != NULL) {
                position_allowed_in_buffer++;
              }
              fprintf(stderr,"packet out of order de seqnum : %d placé dans le buffer a la position : %d\n",pkt_get_seqnum(new_pkt),position_allowed_in_buffer);
              receiving_buffer[position_allowed_in_buffer] = new_pkt;
              send_ack(sfd,window,seq_num_expected);
              fprintf(stderr, "send ack seq_num_expected : %d\n",seq_num_expected);
            }
            else
            {
              fprintf(stderr,"packet reçu DISCARD : num_seq %d\n",pkt_get_seqnum(new_pkt));
              send_ack(sfd,window,seq_num_expected);
            }


          }

        }

      }

    }

  }
  return 0;
}
