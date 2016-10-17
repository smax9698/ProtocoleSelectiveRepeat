#ifndef __CONNECTION_AND_TRANSFER_H_
#define __CONNECTION_AND_TRANSFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>

/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @port: Le port sur lequel le receiver écoute
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 */
const char * real_address(const char *address, uint16_t port,struct sockaddr_in6 *rval);

#endif
