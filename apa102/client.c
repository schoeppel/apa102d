#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
	int sockfd, portno, n;
	unsigned int serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;
	char buf[65536];

	/* check command line arguments */
	if (argc < 2) {
	   fprintf(stderr,"usage: %s <hostname> <key1>=<value1> [... <keyN>=<valueN>]\n", argv[0]);
	   exit(0);
	}

	unsigned int bufpos = 0;
	for (int i = 2; i < argc && bufpos < sizeof(buf); i++) {
		size_t len = strlen(argv[i]) + 1;
		memcpy(buf + bufpos, argv[i], len);
		bufpos += len;
	}

	hostname = argv[1];
	portno = 1910;

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		exit(0);
	}

	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(portno);

	/* send the message to the server */
	serverlen = sizeof(serveraddr);
	n = sendto(sockfd, buf, bufpos, 0, (const struct sockaddr*)&serveraddr, serverlen);
	if (n < 0)
	  error("ERROR in sendto");

	struct pollfd pfd;
	pfd.fd = sockfd;
	pfd.events = POLLIN;

	int ret = poll(&pfd, 1, 200);

	if (ret == 0) {
		printf("timeout\n");
		return 1;
	} else if (ret == 1) {
		n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&serveraddr, &serverlen);
		if (n < 0) error("ERROR in recvfrom");

		for (int i = 0; i < n; i++) {
			if (buf[i] == '\0') buf[i] = ' ';
		}

		printf("%s", buf);

		return 0;
	}
}
