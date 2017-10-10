#include "real_address.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stddef.h>

/*
  int getaddrinfo(const char *node, const char *service,
    const struct addrinfo *hints, struct addrinfo **res);

    struct addrinfo {
                 int              ai_flags;
                 int              ai_family;
                 int              ai_socktype;
                 int              ai_protocol;
                 socklen_t        ai_addrlen;
                 struct sockaddr *ai_addr;
                 char            *ai_canonname;
                 struct addrinfo *ai_next;
             };

ai_family   This field specifies the desired address family for the returned  addresses.   Valid  values  for
    this field include AF_INET and AF_INET6.  The value AF_UNSPEC indicates that getaddrinfo() should
    return socket addresses for any address family (either IPv4 or IPv6, for  example)  that  can  be
    used with node and service.

ai_socktype This  field specifies the preferred socket type, for example SOCK_STREAM or SOCK_DGRAM.  Specify‐
    ing 0 in this field indicates that socket addresses of any type can be returned by getaddrinfo().

ai_protocol This field specifies the protocol for the returned socket addresses.  Specifying 0 in this  field
    indicates that socket addresses with any protocol can be returned by getaddrinfo().

ai_flags  This  field  specifies additional options, described below.  Multiple flags are specified by bit‐
    wise OR-ing them together.

Given  node  and  service,  which  identify an Internet host and a service, getaddrinfo() returns one or more
    addrinfo structures, each of which contains an Internet address that can be specified in a call to bind(2) or
    connect(2).   The getaddrinfo() function combines the functionality provided by the gethostbyname(3) and get‐
    servbyname(3) functions into a single interface, but unlike the latter functions, getaddrinfo() is  reentrant
    and allows programs to eliminate IPv4-versus-IPv6 dependencies.

See errors for the return value in the man.

*/
/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 *          (const char* means the caller cannot modify or free the return value,
 *           so do not use malloc!)
 */
const char * real_address(const char *address, struct sockaddr_in6 *rval){
  struct addrinfo hints, *tmp;

  // The memset() function fills the first n bytes of the memory area pointed
  // to by s with the constant byte c
  memset(rval, 0, sizeof(*rval));
  // define family -> IPV6
  rval->sin6_family = AF_INET6;


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;

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
  if(err != 0) return gai_strerror(err);
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
  return NULL;
}
