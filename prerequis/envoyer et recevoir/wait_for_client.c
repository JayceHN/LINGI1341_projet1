#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <errno.h>
#include <poll.h>
#include "wait_for_client.h"

/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd){
  struct sockaddr_in6 addr;
  char buf[1024];
  int connectfd = 0;
  socklen_t len;
  /*
  ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                      struct sockaddr *src_addr, socklen_t *addrlen);
  recvfrom() places the received message into the buffer buf.  The caller must
  specify the size of  the  buffer in len.

  These calls return the number of bytes received, or -1 if an error occurred.
  In the event of an error, errno is set to indicate the error.

  These  are  some  standard  errors  generated  by  the  socket layer.
  Additional errors may be generated and returned from the underlying protocol
  modules; see their manual pages.
  */
  int rc = setsockopts(connectfd, SOL_SOCKET, SO_BINDTODEVICE, "eth0", 4);
  if(rc < 0) perror(strerror(errno));
  int ret = recvfrom(sfd, buf, sizeof(buf), MSG_PEEK, (struct sockaddr *) &addr, &len);
  if(ret < 0) perror(strerror(errno));

  connectfd = connect(sfd, (struct sockaddr *) &addr, len);
  if(connectfd < 0){
    perror(strerror(errno));
    return -1;
  }
  return 0;
}
