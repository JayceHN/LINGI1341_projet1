#include "sender.h"

int main(int argc, char *argv[])
{
  /*
  * Déclaration et initialisation des variables
  */
  int file_d = fileno(stdin); //le file descriptor dont on aura besoin pour lire le fichier
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
  //    fprintf(stderr, "host: %s\n"
  //                    "port: %d\n", host,port);


  /*
  * Etablissement d'une connexion pour pouvoir communiquer
  * A l'aide des méthodes real_address et create_socket créées auparavant
  */
  int address = real_address(host, &destination);
  if (address != 0){
    fprintf(stderr, "sender.c: Il y a eu un problème avec la méthode real_address.\n");
    return EXIT_FAILURE;
  }
  socket_fd = create_socket(NULL, 0, &destination, port);
  if(socket_fd == -1){
    fprintf(stderr, "sender.c: Il y a eu un problème lors de la création du socket.\n");
    return EXIT_FAILURE;
  }
  //fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK);
  /*
  * Communication à proprement parler
  */

  status = read_write_loop(socket_fd, file_d, &destination);
  if (status < 0){
    fprintf(stderr, "sender.c: la méthode read_write_loop ne s'est pas exécutée comme il faut.\n");
    return EXIT_FAILURE;
  }

  /*
  * Fermeture du canal de communication
  */

  close(socket_fd);

}
int read_write_loop(int socket_fd, int file_d,  struct sockaddr_in6 *dest){
  int status = 0;
  /*
  * Recevoir les données:
  * - lire les données reçues
  * - création d'un paquet
  * - décoder les données reçues
  */
  //déclaration et initialisaton des variables
  char buffer1[MAX_PACKET_SIZE]; //buffer des données que l'on reçoit
  char buffer2[MAX_PAYLOAD_SIZE]; //buffer des données que l'on reçoit
  memset(buffer1, 0, MAX_PACKET_SIZE);
  memset(buffer2, 0, MAX_PAYLOAD_SIZE);

  pkt_t* pkt ; //le paquet qu'on va recevoir
  pkt_t* ack = pkt_new(); //le paquet qu'on va recevoir

  int i;
  int rv = 0;
  int size = 0;
  socklen_t size_in6 = sizeof(struct sockaddr_in6);
  size_t len = MAX_PACKET_SIZE;

  time_t now;
  struct tm *tm;
  uint32_t stamp = 0;

  uint8_t seqnum = 0;

  pkt_t *sender_buffer[WINDOW_SIZE];
  for(i = 0 ; i < WINDOW_SIZE ; i++){
    sender_buffer[i] = NULL;
  }
  uint8_t sender_buffer_size = WINDOW_SIZE;

  int lastack = 0 ;
  int count = 0 ;

  struct pollfd ufds[2];
  ufds[0].fd = socket_fd;
  ufds[0].events = POLLIN;
  ufds[1].fd = file_d;
  ufds[1].events = POLLIN;

  while(42){

    rv = poll(ufds, 2, -1);
    if(rv <= 0) break;


    //fast retransmit
    if (count >= 3) {
      for (i = 0; i < WINDOW_SIZE; i++) {
        if(sender_buffer[i] != NULL && pkt_get_seqnum(sender_buffer[i]) == lastack){

          pkt_encode(sender_buffer[i], buffer1, &len);
          sendto(socket_fd, buffer1, len, 0, (struct sockaddr *) dest, sizeof(struct sockaddr_in6));
          memset(buffer1, 0, MAX_PACKET_SIZE);
          len = MAX_PACKET_SIZE;
          count = 0;
        }
      }
    }

    //réenvoyer
    if (sender_buffer_size < WINDOW_SIZE) {
      now = time(0);
      tm = localtime(&now);
      stamp = tm->tm_sec;

      for (i = 0; i < WINDOW_SIZE; i++) {
        if (sender_buffer[i] != NULL) {
          //RTT
          if ( (double) stamp - (double) pkt_get_timestamp(sender_buffer[i]) > 0.0005 && pkt_get_seqnum(sender_buffer[i]) <= seqnum  ) {
            pkt_encode(sender_buffer[i], buffer1, &len);
            sendto(socket_fd, buffer1, len, 0, (struct sockaddr *) dest, sizeof(struct sockaddr_in6));
            memset(buffer1, 0, MAX_PACKET_SIZE);
            len = MAX_PACKET_SIZE;
          }
        }
      }
    } // end if réenvoyer

    //on lit sur le socket_fd, on reçoit un ack
    if(ufds[0].revents & POLLIN){
      size = recvfrom(socket_fd, buffer1, MAX_PACKET_SIZE, 0, (struct sockaddr *)dest, &(size_in6));

      if(size < 0) break ;

      status = pkt_decode(buffer1, size, ack);
      memset(buffer1, 0, MAX_PACKET_SIZE);
      //ack attendu.
      if (status == PKT_OK && pkt_get_type(ack) == PTYPE_ACK) {
        if(lastack == pkt_get_seqnum(ack)) count++;
        lastack = pkt_get_seqnum(ack);
        //TODO : CHANGER CONDITION SEQNUM
        for (i = 0; i < WINDOW_SIZE; i++) {
          if (sender_buffer[i] != NULL) {
            if (pkt_get_seqnum(ack) > pkt_get_seqnum(sender_buffer[i]) && pkt_get_seqnum(ack) <=  seqnum) {
              pkt_del(sender_buffer[i]);
              sender_buffer[i] = NULL;
              sender_buffer_size ++;
            }
          }
        }
      }// fin ACK
    } //fin poll

    //on lit sur le file_d, données à envoyer
    if (ufds[1].revents & POLLIN) {
      if(seqnum < lastack + sender_buffer_size){
        size = read(file_d, buffer2, MAX_PAYLOAD_SIZE);
        if(size < 0) break;
        if(size == 0 && sender_buffer_size == WINDOW_SIZE) break;
        if(size > 0){
          pkt = pkt_new();
          pkt_set_type(pkt, PTYPE_DATA);
          pkt_set_seqnum(pkt, seqnum);

          now = time(0);
          tm = localtime(&now);
          stamp = tm->tm_sec;
          pkt_set_timestamp(pkt, stamp);

          sender_buffer_size --;
          pkt_set_window(pkt, sender_buffer_size);
          pkt_set_payload(pkt, buffer2, size);
          len = MAX_PACKET_SIZE;
          pkt_encode(pkt, buffer1, &len);
          sendto(socket_fd, buffer1, len, 0, (struct sockaddr *) dest, sizeof(struct sockaddr_in6));
          for ( i = 0; i < WINDOW_SIZE; i++) {
            if (sender_buffer[i] == NULL) {
              sender_buffer[i] = pkt;
              seqnum = inc_seqnum(seqnum);
              break;
            }
          }

          memset(buffer1, 0, MAX_PACKET_SIZE);
          memset(buffer2, 0, MAX_PAYLOAD_SIZE);
          len = MAX_PACKET_SIZE;
        }
      }
    }

    else if(file_d != fileno(stdin) && sender_buffer_size == WINDOW_SIZE){
      pkt = pkt_new();
      pkt_set_type(pkt, PTYPE_DATA);
      pkt_set_seqnum(pkt, seqnum);

      now = time(0);
      tm = localtime(&now);
      stamp = tm->tm_sec;
      pkt_set_timestamp(pkt, stamp);

      sender_buffer_size --;
      pkt_set_window(pkt, sender_buffer_size);
      pkt_set_length(pkt, 0);
      pkt_encode(pkt, buffer1, &len);
      sendto(socket_fd, buffer1, len, 0, (struct sockaddr *) dest, sizeof(struct sockaddr_in6));

      sender_buffer[(seqnum % (WINDOW_SIZE - 1))] = pkt;
      seqnum = inc_seqnum(seqnum);

      memset(buffer1, 0, MAX_PACKET_SIZE);
      memset(buffer2, 0, MAX_PAYLOAD_SIZE);
      len = MAX_PACKET_SIZE;
    }
  }// end while 42

  pkt_del(ack);
  close(file_d);
  close(socket_fd);
  return 0;
}
