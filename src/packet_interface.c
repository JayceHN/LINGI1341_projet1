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
  uint8_t tr:1;
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

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
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
  uint16_t header_part1 = 0; // 16 first bits (window, tr, type, seqnum)
  uint16_t length = 0; // 16 next bits (length)
  uint32_t timestamp = 0; // 32 next bits
  uint32_t crc1 = 0; // 32 last bits
  uint32_t crc2 = 0; // 32 last bits
  uint32_t tmp = 0;

  //retrieves the header
  memcpy(&header_part1, data, sizeof(uint16_t));
  header_part1 = ntohs(header_part1); //ntwrk -> host
  memcpy(pkt, &header_part1, sizeof(uint16_t));

  memcpy(&length, data + sizeof(uint16_t) , sizeof(uint16_t));
  length = ntohs(length); //ntwrk -> host
  pkt_set_length(pkt, length);

  memcpy(&timestamp, data + sizeof(uint32_t) , sizeof(uint32_t));
  timestamp = ntohl(timestamp); //ntwrk -> host
  pkt_set_timestamp(pkt, timestamp);

  uint8_t tr = pkt_get_tr(pkt);
  pkt_set_tr(pkt, 0);
  //checking tr bit
  if(tr > 0 && pkt_get_type(pkt) != PTYPE_DATA){
      fprintf(stderr, "Le tr n'est pas valide.\n");
    return E_TR;
  }

  //checking the type
  if(pkt_get_type(pkt) != PTYPE_DATA && pkt_get_type(pkt) != PTYPE_ACK
    && pkt_get_type(pkt) != PTYPE_NACK){
      fprintf(stderr, "Le type n'est pas valide.\n");
    return E_TYPE;
  }

  int block32 = 3;
  if(tr == 0) block32 ++;

  //checking length
  if(pkt_get_type(pkt) == PTYPE_DATA &&
    (  pkt_get_length(pkt) <= 0 || pkt_get_length(pkt) > 512)){
    fprintf(stderr, "La taille n'est pas correcte.\n");

    return E_LENGTH;
  }

  //extract crc1
  memcpy(&tmp, data + 2* sizeof(uint32_t), sizeof(uint32_t));
  tmp = ntohl(tmp);
  pkt_set_crc1(pkt, tmp);
  tmp = 0;
  crc1 = crc32(crc1, (Bytef *) pkt, sizeof(uint32_t) * 2);

  if(crc1 != pkt_get_crc1(pkt)){
    fprintf(stderr, "Le crc1 n'est pas valide, reçu :  %d recalculé : %d\n", pkt_get_crc1(pkt), crc1);
    return E_CRC;
  }


  pkt_set_tr(pkt, tr); // real tr read
  //no payload
  if(pkt_get_tr(pkt) > 0){
    return PKT_OK;
  }

  //extract crc2
  memcpy(&crc2, data + 2*sizeof(uint32_t) + len , sizeof(uint32_t));
  crc2 = ntohl(crc2);
  pkt_set_crc2(pkt, crc2);

  tmp = crc32(tmp, (Bytef *)data + 3 * sizeof(uint32_t) , pkt_get_length(pkt));
  if(tmp != pkt_get_crc2(pkt)){
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
    size_t size = 0;
    uint16_t length = htons(pkt_get_length(pkt));
    uint32_t timestamp = pkt_get_timestamp(pkt);
    //pkt_set_length(pkt,htons(pkt_get_length(pkt)));

    // win + tr + type + seqnum
    memcpy(buf+size, pkt, sizeof(uint16_t));
    size = size + sizeof(uint16_t);

    memcpy(buf+size, &length, sizeof(uint16_t));
    size = size + sizeof(uint16_t);

    //timestamp
    memcpy(buf+size, &timestamp, sizeof(uint32_t));
    size = size + sizeof(uint32_t);

    //compute crc
    uint32_t crc1 = 0;
    crc1 = crc32(crc1, (Bytef *) buf, sizeof(uint32_t) * 2);
    crc1 = htonl(crc1);
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
