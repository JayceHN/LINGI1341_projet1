#ifndef __TRANSPORT_INTERFACE_H_
#define __TRANSPORT_INTERFACE_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
 #include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <sys/select.h>
#include <fcntl.h>

#define MAX_PACKET_SIZE 640
#define TRUE 1
#define FALSE 0

/* Resolve the resource name to an usable IP address
 * @address: The name to resolve
 * @rval: Where the resulting IP address descriptor should be stored
 * @return: 0 if succeeded or -1 in case of error
 */
int real_address(const char *address, struct sockaddr_in6 *rval);


/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port);

/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd);

/*
* Sender is reading stdin and converting read data into packages
* packages are send to the *dest adress using the selective repeat technique
* @sfd : a file descriptor
* @dest : the destination addresss to which the socket should send data
*/
void sender_loop(int sfd, struct sockaddr_in6 *dest, const char *fname);

/*
* Receiver is listening on its own src address and receiving data
* on the sfd file desciptor. Data is decoded and acknowledgments are
* send back if data is acceptable (using selective repeat principle)
* @sfd : a file descriptor
* @src : the src address to which receiver is listening
*/
void receiver_loop(int sfd, struct sockaddr_in6 *src, const char *fname);


/*
* incrementing the sequence and making sure no overflow occurs
*/
uint8_t inc_seqnum(uint8_t seqnum);


/*
* comparing two sequence numbers and returning 0 if seqnum1 < seqnum2
* 1 if seqnum1 = seqnum2 and -1 otherwise
*/
int compare_seqnum(uint8_t seqnum1, uint8_t seqnum2);


#endif
