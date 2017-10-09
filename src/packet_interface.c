#include "packet_interface.h"

//status code returned
pkt_status_code code;

//defining the structure
struct __attribute__((__packed__)) pkt{
  ptypes_t type ; //: 2 DATA, ACK or NACK (1, 2, 3 resp.)
  uint8_t TR ; //: 1 0 or 1 (1 ssi DATA) -> envoi nack
  uint8_t window : 5; // [0,31]
  uint8_t seqnum; // [0,255]
  uint16_t length; // [0,512]
  uint32_t timestamp; // Own iplementation
  uint32_t crc1; //CRC32 on header (TR 0) avant d'être envoyé
  char *payload; // DATA 512 max si TR == 0 length (si non length == null)
  uint32_t crc2; //CRC32 on payload avant d'être envoyé
}

/*
* @pre -
* @post return null if not enough space, return a pointer to the allocated array for a pkt_t
*/
pkt_t *pkt_new(){
  pkt_t *packet = (pkt_t *) calloc(1, sizeof(pkt_t));
  return packet;
}
