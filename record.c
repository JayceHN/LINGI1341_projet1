#include "record.h"
#include <string.h>
#include <arpa/inet.h>


int record_init(struct record *r)
{
	/* Assert r is allocated */
	if (!r) return 1;

	/* Initialize to 0 */
	r->TYPE = 0;
	r->LENGTH = 0;
	r->F = 0;

	/* Postconditions */
	if(record_get_type(r) == 0 && record_get_length(r) == 0
			&& record_has_footer(r) == 0)
				return 0;
	/* Postconditions does'nt pass */
	return 1;
}

void record_free(struct record *r)
{
	free(r->PAYLOAD);
}

int record_get_type(const struct record *r)
{
	return r->TYPE;
}

void record_set_type(struct record *r, int type)
{
	r->TYPE = type;
}

int record_get_length(const struct record *r)
{
	return r->LENGTH;
}

int record_set_payload(struct record *r,
			 const char * buf, int n)
{
	/* allocate n bytes */
	r->PAYLOAD = malloc(n);
	/* assertion on allocation */
	if(!r->PAYLOAD) return 1;

	/* save the new length */
	r->LENGTH = n;

	/* copy n bytes  from buf to r->PAYLOAD */
	memcpy(r->PAYLOAD, buf, n)

	return 0;
}

int record_get_payload(const struct record *r,
			 char *buf, int n)
{
	/* buffer not allocated -> 0 bytes copied */
	if(!buf) return 0;

	/* if the real size is less than the given size */
	int size = n;
	if(record_get_length(r) < n) size = record_get_length(r);

	/* copy size bytes from r->PAYLOAD to buff */
	memcpy(buf, r->PAYLOAD, size);
	return size;

}

int record_has_footer(const struct record *r)
{
	return r->F;
}

void record_delete_footer(struct record *r)
{
	r->FOOTER = 0;
	r->F = 0;
}

void record_set_uuid(struct record *r, unsigned int uuid)
{
	r->F = 1;
	r->FOOTER = uuid;
}

unsigned int record_get_uuid(const struct record *r)
{
	return r->FOOTER; /* 0 is an invalid UUID */
}

int record_write(const struct record *r, FILE *f)
{
	int wrote = 0;

	/* Initialize */
	uint16_t type = record_get_type(r);
	uint8_t footer = record_get_footer(r);
	uint16_t length = record_get_length(r);
	uint16_t length_ntw = ntohs(record_get_length(r)); //network byte order to host byte order
	uint16_t header = 0;

	/* get back the header in one block */
	header = header | footer;
	header <<= 15 ;
	header = header | type;

	/* get the payload */
	uint32_t uuid;
	char *buf = malloc(length);
	if(footer > 0) uuid = record_get_uuid(r);
	if(length > 0){
		if(!buf) return -1;
		record_get_payload(r, buf, length);
	}

	/* write  */
	wrote += 2 * fwrite(&header, 2, 1, f);
	wrote += 2 * fwrite(&length_ntw, 2, 1, f);
	if(length > 0) wrote += length * fwrite(buf, length, 1, f);
	if(footer > 0) wrote += 4 * fwrite(&uuid, 4, 1,f);

 /* N.B. :
	fwrite() return the number of items read or written.  This number equals the
	number of bytes transferred only when size is 1.
 */

	return wrote;
}

int record_read(struct record *r, FILE *f)
{
	/* Initialize */
	record_init(r);
	int read = 0;
	uint16_t header = 0;
	uint16_t length_ntw = 0;

	/* read */
	read += 2 * fread(&header, 2, 1, f);
	read += 2 * fread(&length_ntw, 2, 1, f);
	uint16_t length = htons(length_ntw); //prepare to send data read on network
	record_set_type(r, header);
	header >>=15;
	r->LENGTH = length;

	if(length > 0){
		char *buf = malloc(length);
			if(!buf) return -1;
		read += length * fread(buf, length, 1, f);
		record_set_payload(r, buf, length);
		free(buf);
	}

	if(header > 0){
		uint32_t uuid = 0;
		read += 4 * fread(&uuid, 4, 1, f);
		record_set_uuid(r, uuid);
	}

	/* N.B. :
	 fread() return the number of items read or written.  This number equals the
	 number of bytes transferred only when size is 1.
	*/

	return read;
}
