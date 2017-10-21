#include "packet_interface.c"
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
    if (file_d != 0){
        host = argv[3];
        port = atoi(argv[4]);
    }
    else{
        host = argv[1];
        port = atoi(argv[2]);
    }
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
     * Communication à proprement parler
     */

     status = read_write_loop(socket_fd, file_d);
     if (status < 0){
         fprintf(stderr, "sender.c: la méthode read_write_loop ne s'est pas exécutée comme il faut.\n");
         return EXIT_FAILURE;
     }

    /*
     * Fermeture du canal de communication
     */

     close(socket_fd);

}
int read_write_loop(int socket_fd, int file_d){
    int status = 0;
    while(42){
      /*
       * Envoyer les données si les fenêtres de réception et d'envoi le permettent:
       */
       if (receiver_window > 0 && sender_window > 0){
          status =  send_data(file_d, socket_fd);
          if (status < 0){
              fprintf(stderr, "read_write_loop: la méthode send_data ne s'est pas exécutée correctement.\n");
              return status;
          }
       }

      /*
       * Recevoir les données:
       */
       status = receive_data(socket_fd);
       if (status < 0){
           fprintf(stderr, "read_write_loop: la méthode receive_data ne s'est pas exécutée correctement.\n");
           return status;
       }

    }
    return status;

}

int receive_data(int socket_fd){
    /*
     * Recevoir les données:
     * - lire les données reçues
     * - création d'un paquet
     * - décoder les données reçues
     */
    //déclaration et initialisaton des variables
    char buffer[MAX_PACKET_SIZE]; //buffer des données que l'on reçoit
    size_t data_len; //la taille des données que l'on reçoit
    fd_set descriptor; //file descriptor de lecture
    int status=0; //pour mesurer le bon fonctionnement des méthodes
    pkt_t* pkt = NULL; //le paquet qu'on va recevoir

    FD_ZERO(&descriptor);
    FD_SET(socket_fd,&descriptor);


    // lire les données reçues
    status = select(socket_fd+1, &descriptor, NULL, NULL, 0);
    if(status < 0){
        fprintf(stderr, "receive_data: la méthode select ne s'est pas bien exécutée.\n");
        return EXIT_FAILURE;
    }
    if (FD_ISSET(socket_fd, &descriptor)){
        data_len = read(socket_fd, buffer, sizeof(buffer));
    }
    if (data_len <= 0){
        fprintf(stderr,"receive_data: on n'a pas lu de données.\n");
        return EXIT_FAILURE;
    }

    // création d'un paquet
    pkt = pkt_new();
    if (pkt == NULL){
        fprintf(stderr,"receive_data: la création du paquet s'est mal passée.\n");
        return EXIT_FAILURE;
    }

    // décoder les données reçues
    status = pkt_decode(buffer, data_len, pkt);
    if(status < 0){
        fprintf(stderr,"receive_data: il y a eu un problème lors du décodage.\n");
        return EXIT_FAILURE;
    }
    if (pkt->type == PTYPE_ACK){
        receiver_window = pkt_get_window(pkt);
        pkt_del(pkt_sent[pkt_get_seqnum(pkt)]->pkt);
    }
    else if (pkt->type == PTYPE_NACK){

    }
    pkt_del(pkt);

}

int send_data(int file_d, int socket_fd){
    /*
     * Envoyer les données si les fenêtres de réception et d'envoi le permettent:
     * - lire les données à envoyer
     * - créer un paquet
     * - encoder le paquet
     * - écrire le paquet sur le socket
     * - mettre à jour les informations du sender
     * - garder en mémoire les paquets qui ont été envoyés
     */
     //déclaration et initialisaton des variables
     char buffer[MAX_PAYLOAD];//buffer de la lecture des données
     char buf_pkt[MAX_PACKET_SIZE]; //buffer pour écrire les données sur le pkt
     int status; //status utilisé pour connaitre les erreurs
     fd_set descriptor; //file descriptor de lecture
     FD_ZERO(&descriptor);
     FD_SET(STDIN_FILENO, &descriptor);
     size_t data_len; //taille des données lues et à écrire
     size_t pkt_len = MAX_PACKET_SIZE; // taille du paquet final
      pkt_t* pkt = NULL;//le paquet qu'on va envoyer

     //lecture des données à envoyer
     if (file_d != 0){
         FD_SET(file_d, &descriptor);
     }
     status = select(socket_fd+1, &descriptor, NULL, NULL, 0);
     if (status < 0){
         fprintf(stderr,"send_data: la méthode select n'a pas été exécutée jusqu'au bout.\n");
         return EXIT_FAILURE;
     }
     //on vérifie si le file descriptor est bien contenu dans un ensemble
     if (FD_ISSET(STDIN_FILENO, &descriptor)) {
            data_len = read(STDIN_FILENO,buffer, sizeof(buffer));
     }
     if (FD_ISSET(file_d,&descriptor)) {
            data_len = read(file_d, buffer, sizeof(buffer));
     }
     if(data_len <= 0){
         fprintf(stderr,"send_data: on n'a pas lu de données.\n");
         return EXIT_FAILURE;
     }

     //création et remplissage du paquet
    pkt = pkt_new();
     if(pkt == NULL){
         fprintf(stderr,"send_data: la création du paquet s'est mal passé\n");
         return EXIT_FAILURE;
     }
     //initialisation des champs du paquet
     pkt_set_type(pkt, PTYPE_DATA);
     pkt_set_window(pkt,sender_window);
     pkt_set_tr(pkt,0);
     pkt_set_timestamp(pkt,0);
     pkt_set_payload(pkt, buffer, data_len);//qui se charge de noter la taille
     pkt_set_seqnum(pkt,seqnum_sent);

     //encodage du paquet
     status = pkt_encode(pkt, buf_pkt, &pkt_len);
     if (status < 0){
         fprintf(stderr, "send_data: l'encodage ne s'est pas bien passé.\n");
         return EXIT_FAILURE;
     }

     //écrire le paquet sur le socket
     status = write(socket_fd, buf_pkt, pkt_len);
     if (status < 0){
         fprintf(stderr, "send_data: l'écriture sur le socket ne s'est pas bien passé.\n");
         return EXIT_FAILURE;
     }

     //maj des infos du sender
     sender_window --; //on réduit la fenêtre d'envoi
     seqnum_sent = seqnum_sent++;
     seqnum_sent = seqnum_sent % 255; //on maj le numéro de séquence

     //garder en mémoire les paquets envoyés
     pkt_sent[seqnum_sent] = malloc(sizeof(struct history_pkt_sent));
     pkt_sent[seqnum_sent]->pkt = pkt;
     gettimeofday(&(pkt_sent[seqnum_sent]->timer),NULL);

}

int real_address(const char *address, struct sockaddr_in6 *rval){
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
}
