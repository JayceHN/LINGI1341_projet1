#include "packet_interface.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  //argc prog_name + hostname/IPV6 + port
  if(argc < 3){
    fprintf(stderr, "Pas assez d'arguments : hostname/IPV6 + port\n");
    return -1;
  }
  char *file_name = NULL;
  char *in_addr;
  unsigned int port;
  int socket_descriptor = 0;
  struct sockaddr_in6 in6_addr;

  // option non demandée pour le receiver
  if(strcmp(argv[1], "-f") == 0){
    file_name = argv[2];
    in_addr = argv[3];
    port = atoi(argv[4]);
  }

  // le premier argument est l'addresse du sender
  else{
    in_addr = argv[1];
    port = atoi(argv[2]);
  }

  if(real_address(in_addr, &in6_addr) != 0){
    fprintf(stderr, "Impossible de trouver une addresse pour : %s\n", in_addr);
  }

  socket_descriptor = create_socket(&in6_addr, port, NULL, -1);

  if(socket_descriptor < 0){
    fprintf(stderr, "Impossible de créer un socket \n");
  }

  if(socket_descriptor > 0 && wait_for_client(socket_descriptor) < 0){
    fprintf(stderr, "Impossible de se connecter\n");
  }

  receiver_loop(socket_descriptor, &in6_addr, file_name);

  return EXIT_SUCCESS;

}
