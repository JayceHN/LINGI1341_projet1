#include "transport_interface.h"

int real_address(const char *address, struct sockaddr_in6 *rval){
  struct addrinfo hints, *tmp;

  // The memset() function fills the first n bytes of the memory area pointed
  // to by s with the constant byte c
  memset(rval, 0, sizeof(*rval));
  // define family -> IPV6
  rval->sin6_family = AF_INET6;


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  // retrieve the address in tmp struct
  int err = getaddrinfo(address, NULL, &hints, &tmp);
  /* addrinfo return 0 on success else an error nbr from these :
    EAI_ADDRFAMILY
    EAI_AGAIN
    EAI_BADFLAGS
    EAI_FAIL
    EAI_FAMILY
    EAI_MEMORY
    EAI_NODATA
    EAI_NONAME
    EAI_SERVICE
    EAI_SOCKTYPE
    EAI_SYSTEM
    ->  gai_strerror() to translate
  */
  if(err != 0) return -1;
  /* return the address
  Address format
        struct sockaddr_in6 {
            sa_family_t     sin6_family;   // AF_INET6
            in_port_t       sin6_port;     // port number
            uint32_t        sin6_flowinfo; // IPv6 flow information
            struct in6_addr sin6_addr;     // IPv6 address
            uint32_t        sin6_scope_id; // Scope ID (new in 2.4)
        };

        struct in6_addr {
            unsigned char   s6_addr[16];   // IPv6 address
        };
  */
  rval = memcpy(rval, tmp->ai_addr, sizeof(struct sockaddr_in6));
  return 0;
}


int create_socket(struct sockaddr_in6 *source_addr, int src_port,
                  struct sockaddr_in6 *dest_addr, int dst_port){
  /*
    int socket(int domain, int type, int protocol);
    socket() creates an endpoint for communication and returns a descriptor.
    The domain argument specifies a communication domain; this selects the protocol family which will be used for
      communication.  These families are defined in <sys/socket.h>.  The currently understood formats include:

      Name                Purpose                          Man page
      AF_UNIX, AF_LOCAL   Local communication              unix(7)
      AF_INET             IPv4 Internet protocols          ip(7)
      AF_INET6            IPv6 Internet protocols          ipv6(7)
      AF_IPX              IPX - Novell protocols
      AF_NETLINK          Kernel user interface device     netlink(7)
      AF_X25              ITU-T X.25 / ISO-8208 protocol   x25(7)
      AF_AX25             Amateur radio AX.25 protocol
      AF_ATMPVC           Access to raw ATM PVCs
      AF_APPLETALK        AppleTalk                        ddp(7)
      AF_PACKET           Low level packet interface       packet(7)
      AF_ALG              Interface to kernel crypto API

      On error, -1 is returned,  and  errno  is  set
       appropriately.
  */
  // to name the socket & connect
  int bindfd = 0;
  int connectfd = 0;
  //descriptor
  int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if(sockfd < 0){
    //descibing the last error
    perror(strerror(errno));
    return -1;
  }
  //@source_addr: if !NULL, the source address that should be bound to this socket
  if(source_addr && src_port > 0){
    source_addr->sin6_family = AF_INET6;
    source_addr->sin6_port = htons(src_port);
    /*
      int bind(int sockfd, const struct sockaddr *addr,
             socklen_t addrlen);
      When  a  socket  is  created  with  socket(2),  it exists in a name space
      (address family) but has no address assigned to it.  bind() assigns the
      address specified by addr to the socket referred to by the file  descrip‐
      tor  sockfd.   addrlen specifies the size, in bytes, of the address
      structure pointed to by addr.  Traditionally, this operation is called
      “assigning a name to a socket”
    */
    bindfd = bind(sockfd, (struct sockaddr *) source_addr, sizeof(struct sockaddr_in6));
    if(bindfd < 0){
      close(sockfd);
      sockfd = -1;
      perror(strerror(errno));
      return -1;
    }
  }
  //@dest_addr: if !NULL, the destination address to which the socket should send data
  //@dst_port: if >0, the destination port to which the socket should be connected
  if(dest_addr && dst_port > 0){
    dest_addr->sin6_port = htons(dst_port);
    /*
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    The connect() system call connects the socket referred to by the file
    descriptor sockfd to the address specified by addr.  The addrlen argument
    specifies the size of addr.  The format of the address in addr is  deter‐
     mined by the address space of the socket sockfd; see socket(2) for further
     details.

    If the connection or binding succeeds, zero is returned.  On error, -1 is
    returned, and errno is  set  appropriately.
    */
    connectfd = connect(sockfd, (struct sockaddr *) dest_addr, sizeof(struct sockaddr_in6));
    if(connectfd < 0){
      close(sockfd);
      sockfd = -1;
      perror(strerror(errno));
      return -1;
    }
  }
  return sockfd;
}

void receiver_loop(int sfd, struct sockaddr_in6 *src, const char *fname){
  /*
  struct pollfd {
    int fd;         // the socket descriptor
    short events;   // bitmap of events we're interested in
    short revents;  // when poll() returns, bitmap of events that occurred
};

   events on multiple sockets simultaneously
   poll() performs a similar task to select(2): it waits for one of a set of
    file descriptors to become ready to perform I/O.
    http://beej.us/guide/bgnet/output/html/multipage/pollman.html + man unix
   */
   struct pollfd ufds[1];
   ufds[0].fd = sfd;
   ufds[0].events = POLLIN;
   //One socket & normal data

   pkt_t *packet, *ack;
   pkt_t *receiver_buffer[WINDOW_SIZE];

   int i;
   for (i = 0; i < WINDOW_SIZE; i++)
     receiver_buffer[i] = NULL;

   pkt_status_code code;
   uint8_t window = WINDOW_SIZE;
   size_t len = MAX_PACKET_SIZE;

   char buffer[MAX_PACKET_SIZE];

   memset(buffer, 0, MAX_PACKET_SIZE);

   int size = 0;
   uint8_t seqnum = 0;

   int receiver_fd;
   socklen_t size_addr = sizeof(struct sockaddr_in6);

   //cas -f receiver
   if(fname[0] != '\0'){
     printf("%s\n", fname );
     receiver_fd = open(fname, O_RDWR | O_CREAT, 0666);
   }
   else{
     receiver_fd = fileno(stdout);
   }
   if (receiver_fd < 0) {
     fprintf(stderr, "[DEBUG] errno %d from receiver_fd\n", errno);
     perror(strerror(errno));
   }

   int rv;
   while(42){
     rv = poll(ufds, 1, 5000);
     if(rv <= 0) break;
     //TODO : verifier socket et read_write_loop
    //data on socket
     if (ufds[0].revents & POLLIN) {
       size = recvfrom(sfd, buffer, MAX_PACKET_SIZE, 0,(struct sockaddr *) src, &size_addr);
       fprintf(stderr, "%d byte(s) reçu(s) sur le socket \n", size);

       if (size < 0) fprintf(stderr, "Impossible de lire sur le socket\n" );

       packet = pkt_new();
       code = pkt_decode(buffer, size, packet);
       memset(buffer, 0, len);

       fprintf(stderr, " Packet type : %u\n", pkt_get_type(packet));
       fprintf(stderr, " Packet TR : %u\n", pkt_get_tr(packet));
       fprintf(stderr, " Packet window : %u\n", pkt_get_window(packet));
       fprintf(stderr, " Packet seqnum : %u\n", pkt_get_seqnum(packet));
       fprintf(stderr, " Packet length : %u\n", pkt_get_length(packet));
       fprintf(stderr, " Packet code : %d\n", code);

       //pkt ok et seqnum non attendu
       if (code == PKT_OK && pkt_get_seqnum(packet) != seqnum && pkt_get_type(packet) == PTYPE_DATA) {
         if(pkt_get_seqnum(packet) <= ((seqnum  + window) % (WINDOW_SIZE - 1)) && pkt_get_seqnum(packet) > seqnum ){
           for (i = 0; i < WINDOW_SIZE; i++) {
             if (receiver_buffer[i] != NULL && pkt_get_seqnum(receiver_buffer[i]) == seqnum) {
               break;
             }
             if(receiver_buffer[i] == NULL){
               // on le stock
               receiver_buffer[i] = packet;
               window --;
               break;
             }
           }
         }
         ack = pkt_new();
         pkt_set_type(ack, PTYPE_ACK);
         pkt_set_window(ack, window);
         pkt_set_seqnum(ack, seqnum);
         pkt_encode(ack, buffer, &len);
         len = MAX_PAYLOAD_SIZE;

         sendto(sfd, buffer, len, 0, (struct sockaddr *) src, sizeof(struct sockaddr_in6));
         pkt_del(ack);
       } // end if(pkt_get_seqnum(packet) != seqnum) et dans le range

       // pkt ok et seqnum attendu
       else if (code == PKT_OK &&  pkt_get_seqnum(packet) == seqnum && pkt_get_type(packet) == PTYPE_DATA ) { // pkt valide
          seqnum = inc_seqnum(seqnum);
           //écris sur le fd
          size = write(receiver_fd, pkt_get_payload(packet), pkt_get_length(packet));
          pkt_del(packet);
          fprintf(stderr, "\n");

          if(size < 0){
            fprintf(stderr, "[DEBUG] errno %d from write\n", errno);
            perror(strerror(errno));
            break;
          }
          if(size == 0) fprintf(stderr, "0 bytes écris \n");

          //prépare l'ack
          ack = pkt_new();
          pkt_set_type(ack, PTYPE_ACK);
          pkt_set_window(ack, window);
          pkt_set_seqnum(ack, seqnum);
          pkt_encode(ack, buffer, &len);
          len = MAX_PAYLOAD_SIZE;

          sendto(sfd, buffer, len, 0, (struct sockaddr *) src, sizeof(struct sockaddr_in6));

           // on regarde s'il y a d'autres pkt valide
          for (i = 0; i < WINDOW_SIZE; i++) {
            if (receiver_buffer[i] != NULL && pkt_get_seqnum(receiver_buffer[i]) == seqnum){
              size = write(receiver_fd, pkt_get_payload(packet), pkt_get_length(packet));

              if(size < 0) fprintf(stderr, "Impossible d'écrire les données\n");
              if(size == 0) fprintf(stderr, "0 bytes écris \n");

              pkt_del(receiver_buffer[i]);
              receiver_buffer[i] = NULL;
              window ++;
              seqnum = inc_seqnum(seqnum);

              i=0;
            }
          } //end for check
          pkt_del(ack);
      } //end elseif(pkt_get_seqnum(packet) == seqnum)

      else if(code == PKT_OK && pkt_get_type(packet) == PTYPE_DATA && pkt_get_length(packet) == 0
      && pkt_get_seqnum(packet) == seqnum){
        fprintf(stderr, "Transmission terminée\n" );
        break;
        } // end elseif (pkt_get_length(packet) == 0 && pkt_get_seqnum(packet) == seqnum)

      else if (code != PKT_OK){
        ack = pkt_new();
        pkt_set_type(ack, PTYPE_ACK);
        pkt_set_window(ack, window);
        pkt_set_seqnum(ack, seqnum);
        pkt_encode(ack, buffer, &len);
        len = MAX_PAYLOAD_SIZE;

        sendto(sfd, buffer, len, 0, (struct sockaddr *) src, sizeof(struct sockaddr_in6));
        pkt_del(ack);
      } // end   else if (code != PKT_OK)

      else if (WINDOW_SIZE - window == 0) {

      }

    } // end POLLIN socket
  } // end while
  //pkt_del(ack);
  close(receiver_fd);
  receiver_fd = -1;
} // end fun

uint8_t inc_seqnum(uint8_t seqnum){
  return (seqnum+1)%256;
}
