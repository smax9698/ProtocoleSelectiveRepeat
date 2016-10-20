#include "selective_repeat_sender.h"

/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 * TO DO : timestamp and resend
 */
int selective_repeat_send(int sfd,FILE * f){

  pkt_t * sending_buffer[31]; // buffer contenant les segment à envoyer
  for (size_t i = 0; i < 31; i++) {
    sending_buffer[i] = NULL;
  }

  struct timeval * time_buffer[31];
  for (size_t i = 0; i < 31; i++) {
    time_buffer[i] = NULL;
  }


  uint8_t window = 5; // window [0,31]
  uint8_t max_window = 5;
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
        n = recv(sfd, buf_acknowledgment, 12, 0); // lis 12 bytes (taille d'un acknowledgment)
        printf("reception d'un ack de taille %d\n",n);
        if(n == -1) // message d'erreur en cas d'erreur
        {
          fprintf(stderr, "%s\n",strerror(errno));
        }
        else if(n!=0) // traitement de l'acknowledgment
        {
          // decodage du ack. buf_acknowledgment -> pkt_ack
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
              printf("change window size : %d\n",max_window);
            }

            lastack = pkt_get_seqnum(pkt_ack);
            printf("seq_num de l'ack : %d\n",lastack);

            // retirer les packets acknowledged
            for (size_t i = 0; i < max_window; i++){

              if(sending_buffer[i] != NULL){
                uint16_t seq_num_packet = pkt_get_seqnum(sending_buffer[i]);
                // verifie s'il faut retirer
                if((lastack > seq_num_packet) && ((lastack - seq_num_packet) <= max_window)){
                  pkt_del(sending_buffer[i]);
                  printf("retrait du packet num_seq : %d lastack : %d\n",seq_num_packet,lastack);
                  sending_buffer[i] = NULL;
                  time_buffer[i] = NULL;
                  window++;
                }
                else if((lastack < seq_num_packet) && (lastack + 255 - seq_num_packet) <= max_window){
                  pkt_del(sending_buffer[i]);
                  printf("retrait du packet num_seq : %d de la positio : %d car ack : %d\n",seq_num_packet,i,lastack);
                  sending_buffer[i] = NULL;
                  time_buffer[i] = NULL;
                  window++;
                }

              }
            }
          }
        }
      }

      // envoi des packets
      if(FD_ISSET(sfd,&write_fd)){

          // tant qu'il y a de la place dans le buffer, envoyer des packets.
          if(window > 0 && n != 0){

            memset(buf_payload,0,512);
            n = read(fd,buf_payload,512); // lecture de max 512 bytes pour mettre dans le payload
            printf("lecture de %d bytes\n",n);

            if(n != 0){

              // encodage de buf_payload -> new_pkt
              pkt_t * new_pkt = pkt_new();

              pkt_set_length(new_pkt,n);
              pkt_set_type(new_pkt,PTYPE_DATA);

              pkt_set_seqnum(new_pkt,seq_num);
              seq_num = (seq_num+1)%256; // next seq_num

              pkt_set_window(new_pkt,window);
              pkt_set_timestamp(new_pkt,1); // TO DO
              pkt_set_payload(new_pkt,buf_payload,n);

              pkt_status_code status_code;
              size_t len = n+12;

              memset(buf_packet,0,524);
              status_code = pkt_encode(new_pkt,buf_packet,&len);

              if(status_code != PKT_OK){
                fprintf(stderr, "Not able to encode the structure\n");
                return -1;
              }

              // recherche d'une position dans le buffer et placer le packet
              uint8_t position_allowed_in_buffer = 0;
              while(position_allowed_in_buffer < max_window && sending_buffer[position_allowed_in_buffer] != NULL) {
                position_allowed_in_buffer++;
              }

              sending_buffer[position_allowed_in_buffer] = new_pkt;
              printf("packet placé dans le buffer à la position : %d\n",position_allowed_in_buffer);
              window--;
              printf("taille dispo dans le buffer : %d\n",window);
              err = send(sfd,buf_packet,len,0);

              printf("packet envoyé\n");

              // enregistre le moment d'envoi

              struct timeval * new_time = (struct timeval*)malloc(sizeof(struct timeval));
              gettimeofday(new_time,NULL);
              time_buffer[position_allowed_in_buffer] = new_time;

              if(err == -1){
                fprintf(stderr, "send error %s\n",strerror(errno));

              }
            }
        }

          // tester les packets à renvoyer

      }


      // Cherche les paquets perdu (ou considéré comme) et send

      for (size_t i = 0; i < 31; i++) {

        if(time_buffer[i] != NULL){

          struct timeval * now = (struct timeval *)malloc(sizeof(struct timeval));
          gettimeofday(now,NULL);

          if(now->tv_sec - time_buffer[i]->tv_sec > 4){

              size_t len = pkt_get_length(sending_buffer[i])+12;
              memset(buf_packet,0,524);
              pkt_status_code status_code = pkt_encode(sending_buffer[i],buf_packet,&len);
              if(status_code != PKT_OK){
                fprintf(stderr, "Not able to encode the structure\n");
                return -1;
              }

              err = send(sfd,buf_packet,len,0);
              gettimeofday(time_buffer[i],NULL);
          }
          free(now);
        }
      }

    }
  }
  return 0;
}
