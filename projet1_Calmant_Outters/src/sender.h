#ifndef __SENDER_INTERFACE_H_
#define __SENDER_INTERFACE_H_

#include "packet_interface.h"
#include "transport_interface.h"
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/time.h>

#define MIN_ARGUMENT 3
#define MAX_ARGUMENT 5
#define MAX_PAYLOAD 512
#define MAX_SEQNUM 5
//#define MAX_PACKET_SIZE 528 // taille max du payload + 4 octets header + 4 octets timestamp + 8 octets des deux crc


uint8_t sender_window = WINDOW_SIZE;
uint8_t receiver_window = WINDOW_SIZE;
uint8_t seqnum_sent = 0;

struct history_pkt_sent{
  struct timeval timer;
  pkt_t* pkt;
};

struct history_pkt_sent* pkt_sent[WINDOW_SIZE];

int read_write_loop(int socket_fd, int file_d);
int send_data(int file_d, int socket_fd);
int receive_data(int socket_fd);

#endif
