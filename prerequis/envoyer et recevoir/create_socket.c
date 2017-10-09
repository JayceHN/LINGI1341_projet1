#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <errno.h>
#include "create_socket.h"

/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
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
  int sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0){
    //descibing the last error
    perror(strerror(errno));
    return -1;
  }
  //@source_addr: if !NULL, the source address that should be bound to this socket
  if(source_addr){
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
      perror(strerror(errno));
      return -1;
    }
  }
  //@dest_addr: if !NULL, the destination address to which the socket should send data
  //@dst_port: if >0, the destination port to which the socket should be connected
  if(dest_addr){
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
      perror(strerror(errno));
      return -1;
    }
  }
  return sockfd;
}
