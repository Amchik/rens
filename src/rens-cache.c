#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "include/rens-cache.h"

__inline static uint32_t rensc_ttl(const char *data) {
	struct RensHeader h;
	struct RensQuestion q;
	struct RensResource r;
	size_t off, ttl, i;

	ttl = 0;

	off = 12;
	h = rens_read_header(data);
	for (i = 0; i < h.qdcount; ++i) {
		q = rens_read_question(data + off);
		off += q.qname.len + 2 * sizeof(uint16_t);
	}
	for (i = 0; i < h.ancount; ++i) {
		r = rens_read_resource(data + off);
		off += r.name.len + r.rdata.len + 2 * sizeof(uint16_t) + sizeof(uint32_t);

		if (ttl == 0 || r.ttl < ttl)
			ttl = r.ttl;
	}


	return ttl;
}

rensc_vec rensc_new(size_t cap) {
	rensc_vec v;

	v.cap = cap;
	v.len = 0;
	v.ptr = malloc(sizeof(rensc_obj) * cap);

	return v;
}
rensc_obj *rensc_find(const rensc_vec *vec, struct RensQuestion q) {
	size_t i, j;
	rensc_obj *o;
	struct RensQuestion e;

	for (i = 0; i < vec->len; ++i) {
		o = vec->ptr + i;
		e = rens_read_question((char*)o->buff.ptr + 12);
		if (q.qclass != e.qclass || q.qtype != e.qtype || q.qname.len != e.qname.len)
			continue;
		for (j = 0; j < q.qname.len; ++j)
			if (q.qname.ptr[j] != e.qname.ptr[j])
				continue;
		return o;
	}

	return 0;
}
void rensc_push(rensc_vec *vec, uint8_t *data, size_t len) {
	if (vec->len == vec->cap) {
		rensc_realloc(vec, vec->cap + RENSC_DEFAULT_CAP);
	}

	vec->ptr[vec->len].created = time(0);
	vec->ptr[vec->len].buff.len = len;
	vec->ptr[vec->len].buff.ptr = malloc(len);
	memcpy(vec->ptr[vec->len].buff.ptr, data, len);

	vec->len += 1;
}
void rensc_realloc(rensc_vec *vec, size_t newcap) {
	vec->cap = newcap;
	vec->ptr = realloc(vec->ptr, newcap * sizeof(rensc_obj));
}
void rensc_optimize(rensc_vec *vec) {
	ssize_t i;
	size_t now, ttl;
	rensc_obj o;

	now = time(0);

	for (i = 0; i < vec->len; ++i) {
		o = vec->ptr[i];
		ttl = rensc_ttl((char*)o.buff.ptr);
		if (o.created + ttl < now) {
			free(o.buff.ptr);
			if (i + 1 == vec->len) {
				vec->len -= 1;
			} else {
				memmove(vec->ptr + i, vec->ptr + i + 1, vec->len - i);
				--i;
			}
		}
	}
}

