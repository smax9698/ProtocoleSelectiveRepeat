#include "connection_and_transfer.h"

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
