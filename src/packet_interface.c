#include "packet_interface.h"

//status code returned
pkt_status_code code;

//defining the structure
struct __attribute__((__packed__)) pkt
{
  ptypes_t type ; //: 2 DATA, ACK or NACK (1, 2, 3 resp.)
  uint8_t TR ; //: 1 0 or 1 (1 ssi DATA) -> envoi nack
  uint8_t window : 5; // [0,31]
  uint8_t seqnum; // [0,255]
  uint16_t length; // [0,512]
  uint32_t timestamp; // Own iplementation
  uint32_t crc1; //CRC32 on header (TR 0) avant d'être envoyé
  char *payload; // DATA 512 max si TR == 0 length (si non length == null)
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
  code = PKT_OK;
  return code;
}

pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
{
  code = PKT_OK;
  return code;
}
