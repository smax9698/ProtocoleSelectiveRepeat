#include "selective_repeat_sender.h"

/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 * TO DO : timestamp and resend
 */
int selective_repeat_send(int sfd,FILE * f){

  pkt_t * sending_buffer[31]; // buffer contenant les segment à envoyer
  uint8_t window = 2; // window [0,31]
  uint8_t max_window = 2;
  //uint8_t authorized_buff_max = 1; // taille de la window du receiver
  uint16_t lastack = 0; // dernier numero de sequence acknowledged
  uint16_t seq_num = 0; // numero de sequence actuel

  ssize_t err;
  size_t n = -1; // nombre de bytes lus
  char buf_payload[512]; // payload
  char buf_packet[524]; // packet
  char buf_acknowledgment[12]; // acknowledgement

  int fd = fileno(f);
  int max_fd = sfd+1;

  fd_set read_fd, write_fd;

  while(n != 0) // CHANGER LA CONDITION D'ARRET
  {

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

      // envoi des packets
      if(FD_ISSET(sfd,&write_fd)){

          // tant qu'il y a de la place dans le buffer, envoyer des packets.
          while(window > 0 && n != 0){

            memset(buf_payload,0,512);
            n = read(fd,buf_payload,512); // lecture de max 512 bytes pour mettre dans le payload
            printf("pack\n");
            if(n != 0){
              pkt_t * new_pkt = pkt_new();

              pkt_set_length(new_pkt,n);
              pkt_set_type(new_pkt,PTYPE_DATA);

              seq_num = (seq_num+1)%256;

              pkt_set_seqnum(new_pkt,seq_num);
              pkt_set_window(new_pkt,window);
              pkt_set_timestamp(new_pkt,1); // TO DO
              pkt_set_payload(new_pkt,buf_payload,n);

              pkt_status_code status_code;
              size_t len = 524;

              memset(buf_packet,0,524);
              status_code = pkt_encode(new_pkt,buf_packet,&len);
              if(status_code != PKT_OK){
                fprintf(stderr, "Not able to encode the structure\n");
                return -1;
              }

              uint8_t position_allowed_in_buffer = 0;
              while(position_allowed_in_buffer < max_window && sending_buffer[position_allowed_in_buffer] != NULL) {
                position_allowed_in_buffer++;
              }

              sending_buffer[position_allowed_in_buffer] = new_pkt;
              window--;

              err = send(sfd,buf_packet,sizeof(buf_packet),0);

              if(err == -1){
                fprintf(stderr, "send error %s\n",strerror(errno));

              }
            }
        }

        // réception des acknowledgments
        if(FD_ISSET(sfd,&read_fd)){

          n = recv(sfd, buf_acknowledgment, 12, 0); // lis 12 bytes (taille d'un acknowledgment)
          printf("ack\n");
          if(n == -1) // message d'erreur en cas d'erreur
          {
            fprintf(stderr, "%s\n",strerror(errno));
          }
          else if(n!=0) // traitement de l'acknowledgment
          {

            pkt_t *pkt_ack = pkt_new();

            pkt_status_code status_code;
            status_code = pkt_decode(buf_acknowledgment,12,pkt_ack);

            if(status_code != PKT_OK){
              fprintf(stderr, "acknowledgment rejected\n");
            }
            else{

              // mise a jour de la taille de la window
              if(pkt_get_window(pkt_ack) != max_window){

                uint8_t new_free_place = pkt_get_window(pkt_ack) - max_window;
                window += new_free_place;
                max_window = pkt_get_window(pkt_ack);

              }

              lastack = pkt_get_seqnum(pkt_ack);

              // retirer les packets acknowledged
              for (size_t i = 0; i < max_window; i++){

                if(sending_buffer[i] != NULL){
                  uint16_t seq_num_packet = pkt_get_seqnum(sending_buffer[i]);

                  // verifie s'il faut retirer
                  if((lastack > seq_num_packet) && ((lastack - seq_num_packet) <= max_window)){
                    pkt_del(sending_buffer[i]);
                    sending_buffer[i] = NULL;
                    window++;
                  }
                  else if((lastack < seq_num_packet) && (lastack + 255 - seq_num_packet) <= max_window){
                    pkt_del(sending_buffer[i]);
                    sending_buffer[i] = NULL;
                    window++;
                  }

                }

              }

            }
          }
        }

          // tester les packets à renvoyer


      }

    }
  }
  return 0;
}
