#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "include/server.h"

#define VERSION "1.0.0"

const char USAGE[] =
	"Usage: %s [path to config]\n"
	"       %s -h|--help\n"
	"Default config path is /etc/rens.conf\n"
	"\n"
	"ReNS is a DNS server, that proxies DNS over HTTPS.\n"
	"ReNS v" VERSION " written by github.com/ValgrindLLVM\n"
	"https://github.com/ValgrindLLVM/rens\n";

int main(int argc, char **argv) {
	FILE *fp;
	char* file_buffer;
	size_t file_len;
	const char *config_path;
	struct RensConfig conf;
	struct RensServerContext ctx;
	pthread_mutex_t mut;
	rensc_vec cache;
	pthread_t *thread;

	if (argc == 2) {
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
			printf(USAGE, argv[0], argv[0]);
			return 1;
		}
		config_path = argv[1];
	} else {
		config_path = "/etc/rens.conf";
	}

	fp = fopen(config_path, "r");
	if (!fp) {
		fprintf(stderr,
				"Failed to open file '%s' (configuration file): %s (os error %d)\n",
				config_path, strerror(errno), errno);
		return 2;
	}

	fseek(fp, 0, SEEK_END);
	file_len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	file_buffer = malloc(file_len + 1);
	file_buffer[file_len] = 0;
	fread(file_buffer, 1, file_len, fp);
	fclose(fp);

	conf = rensconf_read(file_buffer, file_len);
	free(file_buffer);

	pthread_mutex_init(&mut, 0);
	ctx.mut = &mut;
	cache = rensc_new(RENSC_DEFAULT_CAP);
	ctx.cache = &cache;
	ctx.conf = &conf;

	printf("Starting server on %s:%s with:\n"
			" DNS Server: %s\n",
			conf.listen_ip, conf.listen_port, conf.dns_server);

	thread = rens_server_start(ctx);
	if (thread == 0) {
		fprintf(stderr, "Failed to start server.\nErrno: %s (os error %d)\n",
				strerror(errno), errno);
		return 3;
	} else {
		printf("Server started\n");
	}

	for (;;) {
		sleep(10);
		pthread_mutex_lock(ctx.mut);
		rensc_optimize(ctx.cache);
		pthread_mutex_unlock(ctx.mut);
	}
}
