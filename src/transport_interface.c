#include "transport_interface.h"

int real_address(const char *address, struct sockaddr_in6 *rval)
{
  return 0;
}

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port)
{
  return 0;

}

int wait_for_client(int sfd)
{
  return 0;

}

void sender_loop(int sfd, struct sockaddr_in6 *dest, const char *fname)
{

}

void receiver_loop(int sfd, struct sockaddr_in6 *src, const char *fname)
{

}

int checkTime(const int time1, const int time2)
{
  return 0;

}

uint8_t incSeqNum(uint8_t seqnum)
{
  return 0;

}

int compareSeqNum(uint8_t seqnum1, uint8_t seqnum2)
{
  return 0;

}
