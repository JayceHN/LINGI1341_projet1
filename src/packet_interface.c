#include "packet_interface.h"

//status code returned
pkt_status_code code;

//defining the structure
struct __attribute__((__packed__)) pkt
{
  ptypes_t type ; //: 2 DATA, ACK or NACK (1, 2, 3 resp.)
  uint8_t tr ; //: 1 0 or 1 (1 ssi DATA) -> envoi nack
  uint8_t window : 5; // [0,31]
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
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
		pkt->tr = tr;
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t win)
{
		pkt->window = win;
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
		pkt->seqnum = seqnum;
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
		pkt->length = length;
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
		pkt->timestamp = timestamp;
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
		pkt->crc1 = crc1;
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data, const uint16_t length)
{
    if(length <= 0){
      code = E_LENGTH;
      return code;
    }
		pkt->payload = calloc(1, (sizeof(char) * length));
		if(pkt->payload == NULL)
		{
			code = E_NOMEM;
			return code;

		}
		memcpy(pkt->payload, data, length);
		pkt_set_length(pkt, length);
		code = PKT_OK;
		return code;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
  pkt->crc2 = crc2;
  code = PKT_OK;
  return code;
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
    code = E_TR;
    return code;
  }

  //checking the type
  if(type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK){
    code = E_TYPE;
    return code;
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
    code = E_LENGTH;
    return code;
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
    code = E_CRC;
    return code;
  }
  crc_tmp = 0;

  pkt_set_tr(pkt, tr); // real tr read
  //no payload
  if(pkt_get_tr(pkt) > 0){
    code = PKT_OK;
    return code;
  }

  //extract crc2
  memcpy(&crc2, data + len - sizeof(uint32_t), sizeof(uint32_t));
  crc2 = ntohl(crc2);
  pkt_set_crc2(pkt, crc2);

  crc_tmp = crc32(crc_tmp, (Bytef *)data + 3 * sizeof(uint32_t) , pkt_get_length(pkt));
  if(crc_tmp != pkt_get_crc2(pkt)){
    code = E_CRC;
    return code;
  }

  code = PKT_OK;
  return code;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
  code = PKT_OK;
  return code;
}
