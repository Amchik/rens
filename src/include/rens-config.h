/* vim: ft=c
 */

#include <stdlib.h>

struct RensConfig {
	char listen_ip[16];
	char listen_port[6];

	char dns_server[16];
	
	/*char cache_path[128];
	char hosts_path[128];*/
};

/* Read rens.conf from buffer
 */
struct RensConfig rensconf_read(const void *ptr, size_t len);

