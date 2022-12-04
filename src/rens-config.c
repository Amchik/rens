#include <stdlib.h>
#include <string.h>

#include "include/rens-config.h"

#define rensconf_parse(key, buff, conf, rcur) \
	if (rensconf_strstarts(buff, #key " ")) { \
		size_t l; \
		l = rcur - sizeof(#key) < sizeof((conf).key) - 1 \
					? (rcur - sizeof(#key)) \
					: sizeof((conf).key) - 1;\
		memcpy((conf).key, (buff) + sizeof(#key), l); \
		(conf).key[l] = 0; \
	}

__inline static void rensconf_default(struct RensConfig *conf) {
	strncpy(conf->listen_ip,   "127.0.0.52",       sizeof(conf->listen_ip));
	strncpy(conf->listen_port, "53",               sizeof(conf->listen_port));
	strncpy(conf->dns_server,  "1.1.1.1",          sizeof(conf->dns_server)); /*cry about it*/
	/*strncpy(conf->hosts_path,  "/etc/hosts",       sizeof(conf->hosts_path));
	strncpy(conf->cache_path,  "/tmp/.cache.rens", sizeof(conf->cache_path));*/
}
__inline static int rensconf_strstarts(const char *str, const char *prefix) {
	size_t i;

	for (i = 0; prefix[i] != '\0'; ++i)
		if (str[i] != prefix[i])
			return 0;
	
	return 1;
}

struct RensConfig rensconf_read(const void *ptr, size_t len) {
	struct RensConfig rens;
	char *cur, buff[512];
	size_t rcur;

	cur = (char*)ptr;
	rcur = 0;
	rensconf_default(&rens);

	while ((cur - (const char*)ptr) < len && *cur != 0) {
		buff[rcur] = *cur;
		cur += 1;
		rcur += 1;
		if ((cur - (const char*)ptr) < len && *cur != '\n' && *cur != '\0')
			continue;
		
		buff[rcur] = 0;

		     rensconf_parse(listen_ip,   buff, rens, rcur)
		else rensconf_parse(listen_port, buff, rens, rcur)
		else rensconf_parse(dns_server,  buff, rens, rcur)
		/*else rensconf_parse(hosts_path,  buff, rens, rcur)
		else rensconf_parse(cache_path,  buff, rens, rcur)*/

		rcur = 0;
		if (*cur != 0)
			cur += 1;
	}

	return rens;
}
