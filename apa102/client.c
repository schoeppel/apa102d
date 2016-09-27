
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>



void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
	int sockfd, portno, n;
	int serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;
	char buf[65536];

	/* check command line arguments */
	if (argc != 3) {
	   fprintf(stderr,"usage: %s <hostname> <key>=<value>\n", argv[0]);
	   exit(0);
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
	n = sendto(sockfd, argv[2], strlen(argv[2])+1, 0, &serveraddr, serverlen);
	if (n < 0)
	  error("ERROR in sendto");

	/* print the server's reply */
	n = recvfrom(sockfd, buf, sizeof(buf), 0, &serveraddr, &serverlen);
	if (n < 0)
	  error("ERROR in recvfrom");
	printf("Echo from server: %s", buf);
	return 0;
}
