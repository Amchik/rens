#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <curl/curl.h>

#include "include/server.h"

typedef struct {
	uint8_t *ptr;
	size_t   len;
	size_t   cap;
} rens_vec;

static size_t curl_readfn(char *ptr, size_t size, size_t nmemb, void *userp) {
	rens_buffer *r;
	size_t nread;

	r = (rens_buffer*)userp;
	nread = size * nmemb < r->len ? size * nmemb : r->len;

	if (r->len == 0)
		return 0;

	memcpy(ptr, r->ptr, nread);
	r->len -= nread;
	r->ptr += nread;

	return nread;
}
static size_t curl_writefn(void *data, size_t size, size_t nmemb, void *userp) {
	rens_vec *r;
	size_t nread;

	r = (rens_vec*)userp;
	nread = size * nmemb < (r->cap - r->len) ? size * nmemb : (r->cap - r->len);

	memcpy(r->ptr + r->len, data, nread);
	r->len += nread;

	return nread;
}

struct RensServerInternalContext {
	struct RensServerContext ctx;
	int                      server_fd;
};

void *rens_server_loop(void *ptr) {
	struct RensServerInternalContext ictx;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	CURL *curl;
	struct curl_slist *headers;
	ssize_t nread;
	char buff[2048], clenbuff[128], url[128];
	rens_buffer rbuff;
	rens_vec    rvec;
	size_t now;
	rensc_obj *obj;
	struct RensHeader h, hr;
	struct RensQuestion q;

	memcpy(&ictx, ptr, sizeof(ictx));
	free(ptr);
	snprintf(url, sizeof(url), "https://%s/dns-query", ictx.ctx.conf->dns_server);
	
	for (;;) {
		peer_addr_len = sizeof(peer_addr);
		nread = recvfrom(ictx.server_fd, buff, sizeof(buff), 0,
				(struct sockaddr *)&peer_addr, &peer_addr_len);
		if (nread == -1)
			continue;

		h = rens_read_header(buff);
		if (h.qdcount == 1) {
			q = rens_read_question(buff + 12);
			pthread_mutex_lock(ictx.ctx.mut);
			obj = rensc_find(ictx.ctx.cache, q);
			if (obj != 0) {
				hr = rens_read_header((char*)obj->buff.ptr);
				hr.id = h.id;
				rens_write_header(&hr, buff);
				sendto(ictx.server_fd, buff, 12, MSG_MORE,
						(struct sockaddr*) &peer_addr,
						peer_addr_len);
				sendto(ictx.server_fd, obj->buff.ptr + 12, obj->buff.len - 12, 0,
						(struct sockaddr*) &peer_addr,
						peer_addr_len);
				pthread_mutex_unlock(ictx.ctx.mut);
				continue;
			}
			pthread_mutex_unlock(ictx.ctx.mut);
		}

		rbuff.ptr = (uint8_t*)buff;
		rbuff.len = nread;

		rvec.ptr = (uint8_t*)buff;
		rvec.len = 0;
		rvec.cap = sizeof(buff);

		sprintf(clenbuff, "Content-Length: %lu", nread);

		curl = curl_easy_init();
		headers = curl_slist_append(0, "Content-Type: application/dns-message");
		headers = curl_slist_append(headers, clenbuff);

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, curl_readfn);
		curl_easy_setopt(curl, CURLOPT_READDATA,     &rbuff);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writefn);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &rvec);

		curl_easy_perform(curl);

		sendto(ictx.server_fd, rvec.ptr, rvec.len, 0,
				(struct sockaddr*) &peer_addr,
				peer_addr_len);

		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);

		h = rens_read_header((char*)rvec.ptr);
		if (h.qdcount == 1 && h.rcode == RENS_RCODE_OK) {
			pthread_mutex_lock(ictx.ctx.mut);
			rensc_push(ictx.ctx.cache, rvec.ptr, rvec.len);
			pthread_mutex_unlock(ictx.ctx.mut);
		}
	}

	return 0;
}

static pthread_t rens_server_loop_thread;
pthread_t *rens_server_start(struct RensServerContext ctx) {
	struct addrinfo hints, *results, *rp;
	struct RensServerInternalContext *ictx;
	int r;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family    = AF_UNSPEC;
	hints.ai_socktype  = SOCK_DGRAM;
	hints.ai_flags     = AI_PASSIVE;

	ictx = malloc(sizeof(*ictx));

	r = getaddrinfo(ctx.conf->listen_ip, ctx.conf->listen_port, &hints, &results);
	if (r != 0)
		return 0;

	for (rp = results; rp != 0; rp = rp->ai_next) {
		ictx->server_fd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (ictx->server_fd == -1)
			continue;

		if (bind(ictx->server_fd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;

		close(ictx->server_fd);
	}

	freeaddrinfo(results);
	if (rp == 0)
		return 0;

	ictx->ctx = ctx;

	pthread_create(&rens_server_loop_thread, 0, rens_server_loop, ictx);

	return &rens_server_loop_thread;
}
