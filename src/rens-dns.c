#include <string.h>

#include "include/rens-dns.h"

struct RensHeader rens_read_header(const char *buff) {
	struct RensHeader h;

	h.id     = (buff[0] << 8) + buff[1];
	h.opcode = (0x78 & buff[2]) >> 3;
	h.rcode  = 0x0F & buff[3];
	h.flags  = 
		  ((buff[2] & 0x80) ? RENS_QR : 0)
		+ ((buff[2] & 0x04) ? RENS_AA : 0)
		+ ((buff[2] & 0x02) ? RENS_TC : 0)
		+ ((buff[2] & 0x01) ? RENS_RD : 0)
		+ ((buff[3] & 0x80) ? RENS_RA : 0);
	
	h.qdcount = (buff[4]  << 8) + buff[5];
	h.ancount = (buff[6]  << 8) + buff[7];
	h.nscount = (buff[8]  << 8) + buff[9];
	h.arcount = (buff[10] << 8) + buff[11];

	return h;
}

size_t rens_write_header(const struct RensHeader *header, char *buff) {
	buff[0] = header->id >> 8;
	buff[1] = header->id & 0x00FF;
	buff[2] =
		  ((header->flags & RENS_QR) ? 0x80 : 0)
		+ (header->opcode << 3)
		+ ((header->flags & RENS_AA) ? 0x04 : 0)
		+ ((header->flags & RENS_TC) ? 0x02 : 0)
		+ ((header->flags & RENS_RD) ? 0x01 : 0);
	buff[3] =
		  ((header->flags & RENS_RA) ? 0x80 : 0)
		+ header->rcode;

	buff[4]  = header->qdcount >> 8;
	buff[5]  = header->qdcount & 0x00FF;

	buff[6]  = header->ancount >> 8;
	buff[7]  = header->ancount & 0x00FF;

	buff[8]  = header->nscount >> 8;
	buff[9]  = header->nscount & 0x00FF;

	buff[10] = header->arcount >> 8;
	buff[11] = header->arcount & 0x00FF;

	return 12;
}


struct RensQuestion rens_read_question(const char *buff) {
	struct RensQuestion q;
	size_t i;

	for (i = 0; buff[i] != 0; i += buff[i] + 1);
	q.qname.ptr = (unsigned char*)buff;
	q.qname.len = i + 1;

	q.qtype  = (buff[i + 1] << 8) + buff[i + 2];
	q.qclass = (buff[i + 3] << 8) + buff[i + 4];

	return q;
}

size_t rens_write_question(const struct RensQuestion *question, char *buff) {
	size_t namelen;

	namelen = question->qname.len;
	memcpy(buff, question->qname.ptr, namelen);

	buff[namelen]     = question->qtype  >> 8;
	buff[namelen + 1] = question->qtype  & 0x00FF;

	buff[namelen + 2] = question->qclass >> 8;
	buff[namelen + 3] = question->qclass & 0x00FF;

	return namelen + 4;
}


struct RensResource rens_read_resource(const char *buff) {
	struct RensResource r;
	size_t i;

	for (i = 0; buff[i] != 0; i += buff[i] + 1);
	r.name.ptr = (unsigned char*)buff;
	r.name.len = i++;

	r.qtype  = (buff[i]     << 8) + buff[i + 1];
	r.qclass = (buff[i + 2] << 8) + buff[i + 3];
	r.ttl    =
		  (buff[i + 4] << 24)
		+ (buff[i + 5] << 16)
		+ (buff[i + 6] << 8)
		+ buff[i + 7];
	r.rdata.len = (buff[i + 8] << 8) + buff[i + 9];
	r.rdata.ptr = (unsigned char*)(buff + i + 10);

	return r;
}

size_t rens_write_resource(const struct RensResource *resource, char *buff) {
	size_t namelen;

	namelen = resource->name.len;
	memcpy(buff, resource->name.ptr, namelen);

	buff[namelen    ] = resource->qtype >> 8;
	buff[namelen + 1] = resource->qtype & 0x00FF;

	buff[namelen + 2] = resource->qclass >> 8;
	buff[namelen + 3] = resource->qclass & 0x00FF;

	buff[namelen + 4] =  resource->ttl >> 24;
	buff[namelen + 5] = (resource->ttl >> 16) & 0xFF;
	buff[namelen + 6] = (resource->ttl >>  8) & 0xFF;
	buff[namelen + 7] =  resource->ttl        & 0xFF;

	buff[namelen + 8] = resource->rdata.len >> 8;
	buff[namelen + 9] = resource->rdata.len & 0xFF;

	memcpy(buff + namelen + 10, resource->rdata.ptr, resource->rdata.len);

	return namelen + 10 + resource->rdata.len;
}

