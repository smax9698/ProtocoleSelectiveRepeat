#include "fonctions_communes.h"

int read_entries(int argc, char *argv[], char *file_name, char * host_name, int * port){

  int c;
  size_t length;

  if(argc != 3 && argc != 5){
    fprintf(stderr, "Incorrect number of arguments (argc = %d, expected : 3 or 5)\n",argc);
    return -1;
  }


  while((c = getopt(argc , argv, "f:")) != -1){
    switch (c) {
      case 'f':
        length = strlen((const char *)optarg)+1;
        file_name = calloc(length,sizeof(char));
        memcpy((void *)file_name,(const void *)optarg,length-1);
        break;
      default:
        fprintf(stderr, "Unexpected option\n");
        break;
    }
  }

  if(argc-1 < optind){
    printf("hostname and/or port missing\n");
    return -1;
  }

  length = strlen(argv[optind])+1;
  host_name = calloc(length,sizeof(char));
  memcpy((void *)host_name,(const void *)argv[optind],length-1);

  int portnum = atoi(argv[optind+1]);
  if(portnum <= 0){
    fprintf(stderr, "Invalid port : %d (hostname and port have to be the last arguments)\n",portnum);
    return -1;
  }
  memcpy((void*)port,(const void *)&portnum,sizeof(int));

  return 0;

}
