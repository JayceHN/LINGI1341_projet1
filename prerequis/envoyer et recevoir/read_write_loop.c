#include <stdio.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include "read_write_loop.h"
/*
int select(int nfds, fd_set *readfds, fd_set *writefds,
                  fd_set *exceptfds, struct timeval *timeout);

select() and pselect() allow a program to monitor multiple file descriptors,
waiting until one or more of the file descriptors become "ready" for some class
of I/O operation (e.g., input possible).  A file descriptor is considered
ready if it is possible to perform a corresponding I/O operation (e.g.,
read(2) without blocking, or a sufficiently small write(2)).

On success, select() and pselect() return the number of file descriptors
contained  in  the  three  returned descriptor sets (that is, the total number
of bits that are set in readfds, writefds, exceptfds) which may be zero if the
timeout expires before anything interesting happens.  On error, -1 is returned,
 and errno is  set to indicate the error; the file descriptor sets are
 unmodified, and timeout becomes undefined.

 int poll(struct pollfd *fds, nfds_t nfds, int timeout);

 poll() performs a similar task to select(2): it waits for one of a set of file descriptors to become ready to
      perform I/O.

On success, a positive number is returned; this is the number of structures which
have nonzero revents fields (in  other  words,  those  descriptors with events
or errors reported).  A value of 0 indicates that the call timed out and no file
descriptors were ready.  On error, -1 is returned, and errno is set appropriately.

*/
/* Loop reading a socket and printing to stdout,
 * while reading stdin and writing to the socket
 * @sfd: The socket file descriptor. It is both bound and connected.
 * @return: as soon as stdin signals EOF
 */
void read_write_loop(int sfd){
  /*
  File descriptor masks of type fd_set can be initialized and tested with FD_CLR(), FD_ISSET(), FD_SET(), and FD_ZERO() macros.

  FD_CLR(fd, &fdset)
  Clears the bit for the file descriptor fd in the file descriptor set fdset.

  FD_ISSET(fd, &fdset)
  Returns a non-zero value if the bit for the file descriptor fd is set in the file descriptor set pointed to by fdset, and 0 otherwise.

  FD_SET(fd, &fdset)
  Sets the bit for the file descriptor fd in the file descriptor set fdset.

  FD_ZERO(&fdset)
  Initializes the file descriptor set fdset to have zero bits for all file descriptors.
  */
  // max segment size
  char buf[1024];
  int err, len;
  fd_set descriptors;

  while(42){
    // clear
    FD_ZERO(&descriptors);
    // init
    FD_SET(sfd, &descriptors);
    FD_SET(fileno(stdin), &descriptors);

    // monitor
    int ret = select(sfd+1, &descriptors, NULL, NULL, 0);
    if(ret < 0) perror(strerror(errno));
    else{
      //data
      if(FD_ISSET(sfd, &descriptors)){
        // read() attempts to read up to count bytes from file descriptor fd into the buffer starting at buf.
        len = read(sfd, buf, sizeof(buf));
        if(len < 0){
          perror(strerror(errno));
          break;
        }
        // attempts to write
        err = write(fileno(stdout), buf, len);
        if(err < 0) perror(strerror(errno));
      }
      // ->
      //   <-
      //data -> stdin
      else if(FD_ISSET(fileno(stdin), &descriptors)){
        len = read(fileno(stdin), buf, sizeof(buf));
        if(len < 0){
          perror(strerror(errno));
          break;
        }
        err = write(sfd, buf, len);
        if(err < 0) perror(strerror(errno));
      }
    }
  }
  return;
}
