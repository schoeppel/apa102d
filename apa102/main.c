#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <poll.h>

#include "apa102.h"
#include "config.h"
#include "bubbles.h"
#include "color.h"
//#include "particles.h"
#include "test.h"
#include "single_color.h"
#include "stroboscope.h"
#include "waves.h"
#include "off.h"

struct effect_specification {
	char* name;
	void* (*step)(void*, const char**, unsigned long long, struct apa102_led*, int, int);
	void (*destroy)(void*);
};

static const struct effect_specification effects[] = {
	{
		.name = "off",
		.step = off_step,
		.destroy = off_destroy
	},
	{
		.name = "test",
		.step = test_step,
		.destroy = test_destroy
	},
	/*{
		.name = "particles",
		.step = particles_step,
		.destroy = particles_destroy
	},*/
	{
		.name = "bubbles",
		.step = bubbles_step,
		.destroy = bubbles_destroy
	}
};

volatile int running = 1;

void sighandler(int sig) {
	running = 0;
}

int usage(const char* name) {
	fprintf(stderr, "Usage: %s\n", name);
	return 1;
}

static void parse_packetbuf(char* buf, int buflen, char** values, unsigned int num_values) {
	memset(values, 0, num_values * sizeof(char*));

	while (buflen > 0 && num_values) {
		*values = buf;
		values++;
		num_values--;

		size_t len = strlen(buf);
		buf += len + 1;
		buflen -= len + 1;
	}
}

int main(int argc, char** argv) {
	int portno = 1910;
	struct sockaddr_in serveraddr, clientaddr;
	char buf[65536];
	unsigned int buf_len = 0;
	char *bufvalptr[256];

	config_init();

	int ret;

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		perror("Could not bind\n");
		return 1;
	}

	struct pollfd pfd;
	pfd.fd = sockfd;
	pfd.events = POLLIN;

	bufvalptr[0] = NULL;

	struct apa102_led* leds = apa102_open();
	void* effect_state = NULL;
	const struct effect_specification* current_effect = &effects[0];

	signal(SIGINT, sighandler);

	while (running) {
		unsigned long long frame_start = time_ns();

		ret = poll(&pfd, 1, 4);
		if (ret < 0) {
			if (errno == EINTR) continue;

			perror("poll");
			return 1;
		} else if (ret > 0) {
			unsigned int clientlen = sizeof(clientaddr);
			int len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &clientaddr, &clientlen);
			if (len < 0) continue;

			printf("Received packet\n");
			if (len > 0) {
				buf_len = len;

				parse_packetbuf(buf, len, bufvalptr, sizeof(bufvalptr) / sizeof(bufvalptr[0]));
				const char* new_effect_name = get_message_value((const char**)bufvalptr, "effect", "off");

				/* switch effect? */
				if (strcmp(new_effect_name, current_effect->name)) {
					current_effect->destroy(effect_state);
					effect_state = NULL;

					for (unsigned int i = 0; i < sizeof(effects)/sizeof(effects[0]); i++) {
						if (strcmp(effects[i].name, new_effect_name) == 0) {
							current_effect = &effects[i];
						}
					}
				}
			}
			ret = sendto(sockfd, buf, buf_len, 0, (struct sockaddr*)&clientaddr, clientlen);
		}

		unsigned long long now = time_ns();
		unsigned long long duration = now - frame_start;

		if (duration < 5000000) {
			usleep((5000000 - duration) / 1000);
		}

		/* call current render function */
		effect_state = current_effect->step(effect_state, (const char**) bufvalptr, frame_start, leds, devconfig->num_leds, 144);
		apa102_sync();
	}

	current_effect->destroy(effect_state);
	effect_state = NULL;

	/* turn off */
	if (leds == NULL) return 1;
	struct apa102_led black = { 0, 0, 0 };

	for (unsigned int i = 0; i < devconfig->num_leds; i++)
		leds[i] = black;

	apa102_sync();
	apa102_close();

	printf("apa102 exiting\n");

	unlink("/tmp/apa102.pid");
}
