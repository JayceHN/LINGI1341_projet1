#include "packet_interface.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintx_t */
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <arpa/inet.h>
#include <stdio.h>


//status code returned
pkt_status_code code;

//defining the structure
struct __attribute__((__packed__)) pkt
{
  uint8_t window:5;
  uint8_t trFlag:1;
  uint8_t type:2;
  uint8_t seqnum; // [0,255]
  uint16_t length; // [0,512]
  uint32_t timestamp; // Own iplementation
  uint32_t crc1; //CRC32 on header (tr 0) avant d'être envoyé
  char *payload; // DATA 512 max si tr == 0 length (si non length == null)
  uint32_t crc2; //CRC32 on payload avant d'être envoyé
};

pkt_t *pkt_new()
{
  pkt_t *packet = (pkt_t *) calloc(1, sizeof(pkt_t));
  return packet;
}

void pkt_del(pkt_t *pkt)
{
  free(pkt->payload);
  free(pkt);
}

ptypes_t pkt_get_type(const pkt_t *pkt)
{
		return pkt->type;
}

uint8_t pkt_get_tr(const pkt_t *pkt)
{
		return pkt->tr;
}

uint8_t  pkt_get_window(const pkt_t *pkt)
{
		return pkt->window;
}

uint8_t  pkt_get_seqnum(const pkt_t *pkt)
{
		return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t *pkt)
{
		return pkt->length;
}

uint32_t pkt_get_timestamp(const pkt_t *pkt)
{
		return pkt->timestamp;
}

uint32_t pkt_get_crc1(const pkt_t *pkt)
{
		return pkt->crc1;
}


const char* pkt_get_payload(const pkt_t *pkt)
{
		return pkt->payload;
}

uint32_t pkt_get_crc2(const pkt_t *pkt)
{
  return pkt->crc2;
}

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
		pkt->type = type;
		return PKT_OK;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
		pkt->tr = tr;
    	return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t win)
{
		pkt->window = win;
		return PKT_OK;
}

pkt_status_c  memset(buf, 0, *len);
ode pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
		pkt->seqnum = seqnum;
		return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
		pkt->length = length;
		return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
		pkt->timestamp = timestamp;
		return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
		pkt->crc1 = crc1;
    	return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data, const uint16_t length)
{
    if(length <= 0){
        fprintf(stderr, "La taille du payload ne peut éxcéder les 512 octects (erreur dans le payload).\n");
        return E_LENGTH;
    }
		pkt->payload = calloc(1, (sizeof(char) * length));
		if(pkt->payload == NULL)
		{
            fprintf(stderr, "Il n'y a pas assez de mémoire disponible.\n");
			return E_NOMEM;
		}
		memcpy(pkt->payload, data, length);
		pkt_set_length(pkt, length);
		return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
      pkt->crc2 = crc2;
      return PKT_OK;
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
{
  //vars
  uint32_t header = 0; // 32 first bits (type, window, seqnum, length)
  uint32_t timestamp = 0; // 32 next bits
  uint32_t crc1 = 0; // 32 last bits
  uint32_t crc2 = 0; // 32 last bits

  //retrieves the header
  memcpy(&header, data, sizeof(uint32_t));
  header = ntohl(header); //ntwrk -> host

  //extract different fields (inversed order)
  uint16_t length = header;
  uint8_t seqnum = header >> 16;
  uint8_t window = (header << 2) >> 28;
  uint8_t tr = (header << 1) >> 29;
  uint8_t type = header >> 29;
  // 0 to 31

  //checking tr bit
  if(tr > 0 && type != PTYPE_DATA){
      fprintf(stderr, "Le tr n'est pas valide.\n");
    return E_TR;
  }

  //checking the type
  if(type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK){
      fprintf(stderr, "Le type n'est pas valide.\n");
    return E_TYPE;
  }

  /* setters */
  pkt_set_type(pkt, type);
  pkt_set_tr(pkt, 0); //ignored to compute the crc
  pkt_set_window(pkt, window);
  pkt_set_seqnum(pkt, seqnum);
  pkt_set_length(pkt, length);

  int block32 = 3;
  if(tr == 0) block32 ++;

  //checking length
  if(type == PTYPE_DATA &&
    (  pkt_get_length(pkt) <= 0 || pkt_get_length(pkt) > 512
    || pkt_get_length(pkt) != len - block32 * sizeof(uint32_t))){
    fprintf(stderr, "La taille n'est pas correcte.\n");

    return E_LENGTH;
  }

  //extract timestamp
  memcpy(&timestamp, data + sizeof(uint32_t), sizeof(uint32_t)); // header already read
  pkt_set_timestamp(pkt, timestamp);

  //extract crc1
  memcpy(&crc1, data + 2* sizeof(uint32_t), sizeof(uint32_t));
  crc1 = ntohl(crc1);
  pkt_set_crc1(pkt, crc1);

  uint32_t crc_tmp = 0;
  crc_tmp = crc32(crc_tmp, (Bytef *)data, sizeof(uint32_t));
  if(crc_tmp != pkt_get_crc1(pkt)){
    fprintf(stderr, "Le crc1 n'est pas valide.\n");
    return E_CRC;
  }
  crc_tmp = 0;

  pkt_set_tr(pkt, tr); // real tr read
  //no payload
  if(pkt_get_tr(pkt) > 0){
    return PKT_OK;
  }

  //extract crc2
  memcpy(&crc2, data + len - sizeof(uint32_t), sizeof(uint32_t));
  crc2 = ntohl(crc2);
  pkt_set_crc2(pkt, crc2);

  crc_tmp = crc32(crc_tmp, (Bytef *)data + 3 * sizeof(uint32_t) , pkt_get_length(pkt));
  if(crc_tmp != pkt_get_crc2(pkt)){
      fprintf(stderr, "Le crc2 n'est pas valide.\n");
      return E_CRC;
  }

  return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
    //check len sufficent
    if(*len < (4 * sizeof(uint32_t) + pkt_get_length(pkt))){
        fprintf(stderr, "Il n'y a pas assez de mémoire disponible.\n");
              return E_NOMEM;
    }

    memcpy(buf, pkt, sizeof(uint8_t));

    //init vars
    uint32_t header = 0;
    uint32_t real_header = 0;
    uint32_t tmp = 0;
    size_t size = 0;

    uint8_t real_tr = pkt_get_tr(pkt);

    //build header
    header = (header | pkt_get_type(pkt)) << 29;
    tmp = 0 << 28;
    header = (header | tmp);
    tmp = pkt_get_window(pkt) << 23;
    header = (header | tmp);
    tmp = pkt_get_seqnum(pkt) << 15;
    header = (header | tmp);
    header = (header | pkt_get_length(pkt));
    // host -> ntwk
    header = htonl(header);

    //compute crc
    uint32_t crc1 = 0;
    crc1 = crc32(crc1, (Bytef *) &header, sizeof(uint32_t));
    crc1 = htonl(crc1);

    real_header = (real_tr << 28) | header;
    memcpy(buf, &real_header, sizeof(uint32_t));
    size = size + sizeof(uint32_t);

    //timestamp
    uint32_t timestamp = 0;
    timestamp = pkt_get_timestamp(pkt);
    memcpy(buf + size, &timestamp, sizeof(uint32_t));
    size += sizeof(uint32_t);

    memcpy(buf+size, &crc1, sizeof(uint32_t));
    size += sizeof(uint32_t);

    if(pkt_get_payload(pkt) != NULL){ // PAYLOAD + CRC2
    // We add the payload to the buffer
    memcpy(buf+size, pkt_get_payload(pkt), pkt_get_length(pkt));
    size = size+pkt_get_length(pkt);	// size of buffer is  increased
    //crc on buffer
    uint32_t crc2 = 0;
    crc2 = crc32(crc2, (Bytef *) pkt_get_payload(pkt), pkt_get_length(pkt));
    crc2 = htonl(crc2);
    memcpy(buf+size, &crc2, sizeof(uint32_t));
    size += sizeof(uint32_t);
    }

    *len = size;

    return PKT_OK;
}
