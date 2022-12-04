#include <pthread.h>

#include "rens-cache.h"
#include "rens-config.h"

/* Rens Server Context.
 * Must lock on `mut` before use `cache`.
 * `conf` is immutable pointer
 */
struct RensServerContext {
	pthread_mutex_t         *mut;
	rensc_vec               *cache;
	const struct RensConfig *conf;
};

/* FOR INTERNAL USE ONLY! DO NOT USE
 */
void      *rens_server_loop(void *ptr);

pthread_t *rens_server_start(struct RensServerContext ctx);

