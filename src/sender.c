#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include "fonctions_communes.h"

int main(int argc,char *argv[]){

  // lecture et stockage des arguments

  char * file_name = NULL;
  char * hostname = NULL;
  int port;
  if(read_entries(argc,argv,file_name,hostname,&port) == -1){
    fprintf(stderr, "Error in entries\n");
    return -1;
  }

  return 0;
}
