#ifndef __SELECTIVE_REPEAT_RECEIVE_H_
#define __SELECTIVE_REPEAT_RECEIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "packet_implem.h"

int send_ack(int sfd, uint8_t window, uint16_t seq_num);

int selective_repeat_receive(int sfd,int fd);

#endif
