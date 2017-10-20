#include "packet_interface.h"
#include "transport_interface.h"
#include "sender.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /*
     * Déclaration et initialisation des variables
     */
    int file_d = 0; //le file descriptor dont on aura besoin pour lire le fichier
    struct sockaddr_in6 destination; // une structure pour garder l'adresse de destination
    int port; //le port sur lequel le receiver écoute
    char* host; //l'adresse du receiver
    int status = 0; //le status pour la réussite ou l'échec des méthodes
    int socket_fd; //le socket grâce auquel la connexion sera effectuée


    /*
     * Gérer les options placées en ligne de commande
     */
    int opt;
    while((opt = getopt(argc, argv, "f:")) != -1) {
      switch(opt) {
        case 'f':
          file_d = open(optarg, O_RDONLY, S_IRUSR);//on crée un file descriptor avec le nom du fichier avec accès en lecture
          break;

        default:
          fprintf(stderr, "Usage: sender HOSTAME PORT [-f FILENAME] \n"
                          "HOSTNAME l'adresse du host\n"
                          "PORT le numéro du port auquel il faut se connecter\n"
                          "-f FILENAME le nom du fichier dont on lit les données\n");
          break;
      }
    }

    //Trop d'arguments
    if (argc > MAX_ARGUMENT){
        fprintf(stderr, "Il y a trop d'arguments.\n");
        fprintf(stderr, "Usage: sender HOSTAME PORT [-f FILENAME] \n"
                        "HOSTNAME l'adresse du host\n"
                        "PORT le numéro du port auquel il faut se connecter\n"
                        "-f FILENAME le nom du fichier dont on lit les données\n");
        return EXIT_FAILURE;
    }


    //Trop peu d'arguments s'il y a un file ou s'il n'y en a pas
    if (argc < MIN_ARGUMENT || (file_d != 0 && argc < MAX_ARGUMENT)){
        fprintf(stderr, "Il n'y a pas assez d'aruments.\n");
        fprintf(stderr, "Usage: sender HOSTAME PORT [-f FILENAME] \n"
                        "HOSTNAME l'adresse du host\n"
                        "PORT le numéro du port auquel il faut se connecter\n"
                        "-f FILENAME le nom du fichier dont on lit les données\n");
        return EXIT_FAILURE;
    }

    //obtention des valeurs pour host et port
    host = argv[1];
    port = atoi(argv[2]);
    fprintf(stderr, "host: %s\n"
                    "port: %d\n", host,port);


    /*
     * Etablissement d'une connexion pour pouvoir communiquer
     * A l'aide des méthodes real_address et create_socket créées auparavant
     */
     status = real_address(host, &destination);
     if (status != 0){
         fprintf(stderr, "sender.c: Il y a eu un problème avec la méthode real_address.\n");
         return EXIT_FAILURE;
     }
     socket_fd = create_socket(NULL, 0, &destination, port);
     if(socket_fd == -1){
         fprintf(stderr, "sender.c: Il y a eu un problème lors de la création du socket.\n");
         return EXIT_FAILURE;
     }

    /*
     * Communication à proprement parler:
     * Notre read_write_loop écrite auparavant
     */

    /*
     * Fermeture du canal de communication
     */

     if (file_d != 0){
         close(file_d);
     }

}

/*int real_address(const char *address, struct sockaddr_in6 *rval){
  struct addrinfo hints, *tmp;

  memset(rval, 0, sizeof(*rval));
  rval->sin6_family = AF_INET6;


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;

  int err = getaddrinfo(address, NULL, &hints, &tmp);

  if(err != 0) return -1;

  rval = memcpy(rval, tmp->ai_addr, sizeof(struct sockaddr_in6));
  return 0;
}


int create_socket(struct sockaddr_in6 *source_addr, int src_port,
                  struct sockaddr_in6 *dest_addr, int dst_port){

  int bindfd = 0;
  int connectfd = 0;
  //descriptor
  int sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0){
    //descibing the last error
    perror(strerror(errno));
    return -1;
  }
  if(source_addr && src_port > 0){
    source_addr->sin6_family = AF_INET6;
    source_addr->sin6_port = htons(src_port);

    bindfd = bind(sockfd, (struct sockaddr *) source_addr, sizeof(struct sockaddr_in6));
    if(bindfd < 0){
      close(sockfd);
      perror(strerror(errno));
      return -1;
    }
  }
   if(dest_addr && dst_port > 0){
    dest_addr->sin6_port = htons(dst_port);

    connectfd = connect(sockfd, (struct sockaddr *) dest_addr, sizeof(struct sockaddr_in6));
    if(connectfd < 0){
      close(sockfd);
      perror(strerror(errno));
      return -1;
    }
  }
  return sockfd;
}*/
