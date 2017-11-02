#include "packet_interface.h"
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  char * file_name = malloc(64);

  int opt;
  while((opt = getopt(argc, argv, "f:")) != -1){
    switch (opt) {
      // on gère le cas où on précise un fichier
      case 'f':
      file_name = optarg;
      break;

    default:
          file_name[0] = '\0';
          fprintf(stderr, "Usage: receiver HOSTAME PORT [-f FILENAME] \n"
                          "HOSTNAME l'adresse du host\n"
                          "PORT le numéro du port auquel il faut se connecter\n"
                          "-f FILENAME le nom du fichier où on écrit les données\n");
          break;
      }
  }

  if(argc > 5){
    fprintf(stderr, "Il y a trop d'arguments\n");
    fprintf(stderr, "Usage: receiver HOSTAME PORT [-f FILENAME] \n"
                    "HOSTNAME l'adresse du host\n"
                    "PORT le numéro du port auquel il faut se connecter\n"
                    "-f FILENAME le nom du fichier où on écrit les données\n");
    return EXIT_FAILURE;
  }

  if(argc < 3){
    fprintf(stderr, "Il y a trop peu d'arguments\n");
    fprintf(stderr, "Usage: receiver HOSTAME PORT [-f FILENAME] \n"
    "HOSTNAME l'adresse du host\n"
    "PORT le numéro du port auquel il faut se connecter\n"
    "-f FILENAME le nom du fichier où on écrit les données\n");
    return EXIT_FAILURE;
  }

  char *in_addr;
  unsigned int port;
  int socket_descriptor;

  if(file_name[0] != '\0'){
    in_addr = argv[3];
    port = atoi(argv[4]);
  }
  // le premier argument est l'addresse du sender
  else{
    in_addr = argv[1];
    port = atoi(argv[2]);
  }
  struct sockaddr_in6 source;
  if(real_address(in_addr, &source) != 0){
    fprintf(stderr, "Impossible de trouver une addresse pour : %s\n", in_addr);
    return EXIT_FAILURE;
  }

  socket_descriptor = create_socket(&source, port, NULL, 0);

  if(socket_descriptor < 0){
    fprintf(stderr, "Impossible de créer un socket \n");
    return EXIT_FAILURE;
  }

  receiver_loop(socket_descriptor, &source, file_name);

  close(socket_descriptor);

  return EXIT_SUCCESS;
}
