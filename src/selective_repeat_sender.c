#include "selective_repeat_sender.h"

/*
 *  @sfd : file descriptor du socket sur lequel on communique
 *  @fd : file descriptor sur lequel on doit lire les données à envoyer
 *
 * TO DO : timestamp and resend
 */
int selective_repeat_send(int sfd,FILE * f){

  uint8_t window = 1; // window [0,31]
  uint8_t max_window = 1;

  bool window_update = false;

  pkt_t * sending_buffer[MAX_WINDOW_SIZE]; // buffer contenant les segment à envoyer
  for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
    sending_buffer[i] = NULL;
  }

  struct timeval * time_buffer[MAX_WINDOW_SIZE];
  for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
    time_buffer[i] = NULL;
  }

  //uint8_t authorized_buff_max = 1; // taille de la window du receiver
  uint16_t lastack = 0; // dernier numero de sequence acknowledged
  uint16_t seq_num = 0; // numero de sequence actuel

  ssize_t err;
  int n_pack = -1; // nombre de bytes lus
  int n_ack = -1;
  char buf_payload[MAX_PAYLOAD_SIZE]; // payload
  char buf_packet[MAX_PAYLOAD_SIZE+12]; // packet
  char buf_acknowledgment[12]; // acknowledgement

  int fd = fileno(f);
  int max_fd = sfd+1;

  fd_set read_fd, write_fd;

  bool stop_condition = false;
  bool stop_send = false;

  while(!stop_condition) // CHANGER LA CONDITION D'ARRET
  {
    FD_ZERO(&read_fd);
    FD_ZERO(&write_fd);

    FD_SET(sfd,&read_fd);
    FD_SET(sfd,&write_fd);

    err = select(max_fd, &read_fd, &write_fd, NULL,NULL);

    if(err == -1){
      fprintf(stderr, "%s\n",strerror(errno));
      return -1;
    }
    else{

      // réception des acknowledgments
      if(FD_ISSET(sfd,&read_fd)){
        n_ack = recv(sfd, buf_acknowledgment, 12, 0); // lis 12 bytes (taille d'un acknowledgment)
        if(n_ack == -1) // message d'erreur en cas d'erreur
        {
          fprintf(stderr, "%s\n",strerror(errno));
          return -1;
        }
        else if(n_ack!=0) // traitement de l'acknowledgment
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
            if(pkt_get_window(pkt_ack) != max_window && !window_update){

              uint8_t new_free_place = pkt_get_window(pkt_ack) - max_window;
              window += new_free_place;
              max_window = pkt_get_window(pkt_ack);
              window_update = true;
            }

            lastack = pkt_get_seqnum(pkt_ack);
            //fprintf(stderr,"ack recu  : %d\n",lastack);

            // retirer les packets acknowledged
            for (size_t i = 0; i < max_window; i++){

              if(sending_buffer[i] != NULL){
                uint16_t seq_num_packet = pkt_get_seqnum(sending_buffer[i]);

                // verifie s'il faut retirer
                if((lastack > seq_num_packet) && ((lastack - seq_num_packet) <= max_window)){
                  pkt_del(sending_buffer[i]);
                  //fprintf(stderr,"retrait du packet seq_num : %d\n",seq_num_packet);
                  sending_buffer[i] = NULL;
                  time_buffer[i] = NULL;
                  window++;
                }
                else if((lastack < seq_num_packet) && (lastack + 255 - seq_num_packet) <= max_window){
                  pkt_del(sending_buffer[i]);
                  //fprintf(stderr,"retrait du packet seq_num : %d\n",seq_num_packet);
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

          // tant qu'il y a de la place dans le buffer et qu'il y a des packets à envoyer, envoyer des packets.
          if(window > 0 && !stop_send)
          {

            memset(buf_payload,0,512);
            n_pack = read(fd,buf_payload,512); // lecture de max 512 bytes pour mettre dans le payload

            if(n_pack == 0) // si fin de fichier
            {
              stop_send = true;
            }
            else if(n_pack == 1 && buf_payload[0] == '\n') // si fin de lecture sur l'entrée standard
            {
              stop_send = true;
            }
            else{

                // encodage de buf_payload -> new_pkt
                pkt_t * new_pkt = pkt_new();

                pkt_set_length(new_pkt,n_pack);
                pkt_set_type(new_pkt,PTYPE_DATA);

                pkt_set_seqnum(new_pkt,seq_num);
                seq_num = (seq_num+1)%256; // next seq_num

                pkt_set_window(new_pkt,window);
                pkt_set_timestamp(new_pkt,1); // TO DO
                pkt_set_payload(new_pkt,buf_payload,n_pack);

                pkt_status_code status_code;
                size_t len = n_pack+12;

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
                window--;

                err = send(sfd,buf_packet,len,0);

                //fprintf(stderr,"packet envoyé seq_num : %d\n",pkt_get_seqnum(new_pkt));

                // enregistre le moment d'envoi

                struct timeval * new_time = (struct timeval*)malloc(sizeof(struct timeval));
                gettimeofday(new_time,NULL);
                time_buffer[position_allowed_in_buffer] = new_time;

                if(err == -1){
                  fprintf(stderr, "send error %s\n",strerror(errno));
                  return -1;
                }
            }
        }

          // tester les packets à renvoyer

      }


      // Cherche les paquets perdu (ou considéré comme) et send
      if(FD_ISSET(sfd,&write_fd)){
        for (size_t i = 0; i < max_window; i++) {

          if(time_buffer[i] != NULL){

            struct timeval * now = (struct timeval *)malloc(sizeof(struct timeval));
            gettimeofday(now,NULL);

            if((now->tv_sec - time_buffer[i]->tv_sec)*1000 - (now->tv_usec - time_buffer[i]->tv_usec)/1000 > 1000){
                fprintf(stderr,"%ld\n",(now->tv_sec - time_buffer[i]->tv_sec)*1000 - (now->tv_usec - time_buffer[i]->tv_usec)/1000);
                size_t len = pkt_get_length(sending_buffer[i])+12;
                memset(buf_packet,0,524);
                pkt_status_code status_code = pkt_encode(sending_buffer[i],buf_packet,&len);
                if(status_code != PKT_OK){
                  fprintf(stderr, "Not able to encode the structure\n");
                  return -1;
                }

                err = send(sfd,buf_packet,len,0);
                //fprintf(stderr, "packet réenvoyé seq_num %d\n",pkt_get_seqnum(sending_buffer[i]));
                gettimeofday(time_buffer[i],NULL);
                if(err == -1){
                  fprintf(stderr, "send error %s\n",strerror(errno));
                  return -1;
                }
            }
            free(now);
          }
        }
      }

      // verifie que tout les packets ont été envoyé et recu
      if(window == max_window && stop_send){
        stop_condition = true;
      }

    }
  }



  // demande de déconnexion et attente de l'ack
  bool disconnected_sender = false;

  while(!disconnected_sender){
    FD_ZERO(&read_fd);
    FD_ZERO(&write_fd);

    FD_SET(sfd,&read_fd);
    FD_SET(sfd,&write_fd);

    err = select(max_fd, &read_fd, &write_fd, NULL,NULL);

    // réception de l'ack
    if(FD_ISSET(sfd,&read_fd)){

      memset(buf_acknowledgment,0,12);
      n_ack = recv(sfd, buf_acknowledgment, 12, 0); // lis 12 bytes (taille d'un acknowledgment)

      pkt_t *pkt_ack = pkt_new();

      pkt_status_code status_code;
      status_code = pkt_decode(buf_acknowledgment,12,pkt_ack);

      if(n_ack == 12 && pkt_get_window(pkt_ack) == 0){
        disconnected_sender = true;
      }
    }
    if(FD_ISSET(sfd,&write_fd) && !disconnected_sender){
      char buf_ack[12];
      if(time_buffer[0] == NULL) // premier envoi du packet
      {

        pkt_t * pkt_disconnect = pkt_new();

        pkt_set_length(pkt_disconnect,0);
        pkt_set_type(pkt_disconnect,PTYPE_DATA);

        pkt_set_seqnum(pkt_disconnect,seq_num);
        pkt_set_window(pkt_disconnect,window);
        pkt_set_timestamp(pkt_disconnect,1); // TO DO

        memset(buf_ack,0,12);
        size_t len = 12;
        pkt_status_code st_code = pkt_encode(pkt_disconnect,buf_ack,&len);

        if(st_code != PKT_OK){
          fprintf(stderr, "Not able to encode the structure\n");
          return -1;
        }

        sending_buffer[0] = pkt_disconnect;
        err = send(sfd,buf_ack,len,0);

        //fprintf(stderr,"packet envoyé disconnect\n");

        // enregistre le moment d'envoi

        struct timeval * new_time = (struct timeval*)malloc(sizeof(struct timeval));
        gettimeofday(new_time,NULL);
        time_buffer[0] = new_time;

        if(err == -1){
          fprintf(stderr, "send error %s\n",strerror(errno));
          return -1;
        }
      }
      else // si pas de ack après n secondes, renvoyer le packet disconnet consideré comme perdu
      {
        struct timeval * now = (struct timeval *)malloc(sizeof(struct timeval));
        gettimeofday(now,NULL);

        if((now->tv_sec - time_buffer[0]->tv_sec)*1000 - (now->tv_usec - time_buffer[0]->tv_usec)/1000 > 1000){
            size_t len = 12;
            memset(buf_ack,0,12);
            pkt_status_code st_code = pkt_encode(sending_buffer[0],buf_ack,&len);

            if(st_code != PKT_OK){
              fprintf(stderr, "Not able to encode the structure\n");
              return -1;
            }

            err = send(sfd,buf_ack,len,0);
            if(err == -1){
              disconnected_sender = true;
            }
            gettimeofday(time_buffer[0],NULL);
        }
        free(now);
      }
    }
  }

  close(fd);
  close(sfd);

  return 0;
}
