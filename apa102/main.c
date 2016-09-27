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
#include "bubbles.h"
#include "color.h"
#include "particles.h"
#include "simple_step.h"
#include "single_color.h"
#include "stroboscope.h"
#include "waves.h"

int usage(const char* name) {
	fprintf(stderr, "Usage: %s\n", name);
	return 1;
}

int running = 1;


static void parse_packetbuf(char* buf, int buflen, char** values, unsigned int num_values) {
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
	char *bufvalptr[256];


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


	while (running) {
		unsigned long long frame_start = time_ns();

		ret = poll(&pfd, 1, 4);
		if (ret < 0) {
			perror("poll");
			return 1;
		} else if (ret > 0) {
			int clientlen = sizeof(clientaddr);
			int len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &clientaddr, &clientlen);
			if (len <= 0) continue;

			printf("Received packet\n");
			parse_packetbuf(buf, len, bufvalptr, sizeof(bufvalptr) / sizeof(bufvalptr[0]));

			ret = sendto(sockfd, buf, len, 0, &clientaddr, clientlen);
		}

		unsigned long long now = time_ns();
		unsigned long long duration = now - frame_start;

		if (duration < 5000000) {
			usleep((5000000 - duration) / 1000);
		}

		/* call current render function */

	}



	/* turn off */
	struct apa102_led* l = apa102_open();
	if (l == NULL) return 1;
	struct apa102_led black;
	black.global = 0xe0 | 1;
	black.r = 0;
	black.g = 0;
	black.b = 0;

	for (int i = 0; i < NUM_LEDS; i++)
		l[i] = black;

	apa102_sync();
	apa102_close();

	printf("apa102 exiting\n");

	unlink("/tmp/apa102.pid");
}
