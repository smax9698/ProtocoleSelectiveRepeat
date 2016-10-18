#ifndef __SELECTIVE_REPEAT_RECEIVE_H_
#define __SELECTIVE_REPEAT_RECEIVE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "packet_implem.h"

int selective_repeat_receive(int sfd,FILE * f);

#endif
