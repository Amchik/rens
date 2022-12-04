/* vim: ft=c
 */

#include <stdlib.h>
#include "rens-dns.h"

typedef struct {
	rens_buffer buff;
	size_t      created;
} rensc_obj;
typedef struct {
	rensc_obj *ptr;
	size_t     cap;
	size_t     len;
} rensc_vec;

#define RENSC_DEFAULT_CAP 64

rensc_vec  rensc_new(size_t cap);
rensc_obj *rensc_find(const rensc_vec *vec, struct RensQuestion q);
void       rensc_push(rensc_vec *vec, uint8_t *data, size_t len);
void       rensc_realloc(rensc_vec *vec, size_t newcap);
void       rensc_optimize(rensc_vec *vec);

