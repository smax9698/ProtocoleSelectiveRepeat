#include "connection_and_transfer.h"

int create_socket(struct sockaddr_in6 *source_addr,struct sockaddr_in6 *dest_addr){

  int sfd, err_num;

  // création du socket
  sfd = socket(PF_INET6,SOCK_DGRAM,IPPROTO_UDP);

  // traitement des erreurs
  if(sfd == -1){
    fprintf(stderr, "create_socket : %s\n",strerror(errno));
    return -1;
  }

  if(source_addr != NULL){

    source_addr->sin6_family = AF_INET6;

    // liaison à l'adresse source
    err_num = bind(sfd,(struct sockaddr*)source_addr,sizeof(struct sockaddr_in6));

    if(err_num != 0){
      fprintf(stderr, "create_socket : %s\n",strerror(errno));
      return -1;
    }
  }

  if(dest_addr != NULL){

    dest_addr->sin6_family = AF_INET6;

    // liaison à l'adresse destination
    err_num = connect(sfd,(struct sockaddr*)dest_addr,sizeof(struct sockaddr_in6));

    if(err_num != 0){
      fprintf(stderr, "create_socket : %s\n",strerror(errno));
      return -1;
    }

  }
  return sfd;
}


const char * real_address(const char *address, uint16_t port,struct sockaddr_in6 *rval){

  int err_num;

  struct addrinfo hints;
  struct addrinfo *res;

  memset(&hints,0,sizeof(hints));

  hints.ai_family = AF_INET6; // IPv6
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  // remplissage de la structure
  if ((err_num = getaddrinfo(address, NULL, &hints, &res)) != 0) {
      return gai_strerror(err_num);
  }

  // cast struct sock_addr en sockaddr_in6.
  if(res->ai_addr != NULL){
    memcpy((void *)rval,(const void *)res->ai_addr,sizeof(struct sockaddr_in6));
    uint16_t port_network_endian = htons(port);
    memcpy((void*)&rval->sin6_port,(const void *)&port_network_endian,sizeof(uint16_t)); // ajout du port à la structure
  }

  freeaddrinfo(res);

  return NULL;
}

int wait_for_client(int sfd){

  char buff[524];
  struct sockaddr_in6 source_addr;
  socklen_t addr_len = (socklen_t) sizeof(struct sockaddr_in6);
  int err_num;
  memset(&source_addr,0,sizeof(source_addr));


  err_num = recvfrom(sfd, buff, 524, MSG_PEEK,(struct sockaddr *) &source_addr, &addr_len);

  if(err_num == -1){
    fprintf(stderr, "wait_for_client : recvfrom error\n");
    return -1;
  }

  err_num = connect(sfd,(struct sockaddr*)&source_addr,sizeof(struct sockaddr_in6));

  if(err_num != 0){
    fprintf(stderr, "wait_for_client : %s\n",strerror(errno));
    return -1;
  }
  return 0;
}
