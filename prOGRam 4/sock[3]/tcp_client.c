/*
 *******************************************************************************
 *
 *  tcp_client.c --
 *	program to send packets to a server (default port 2000)
 *
 *******************************************************************************
 */

#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

/*
 *  Initialize the socket address info struct.
 */

struct sockaddr_in sock_in, temp, from_addr;
int	from_len;

extern	int	errno;

main(argc, argv)
int	argc;
char	**argv;

{
	struct timeval 	timeout;
	register int 	n;
	u_short 	len;
	char 		*cp;
	int 		i, retry, resplen, done = 0;
	int 		dsmask, flags, sockFD;
	char		buf[100],answer[4048];
	char	hostname[100];
	struct	hostent	*h_name;
	struct	servent	*s_name;

	int 		numTimeOuts	= 0;

	sockFD = -1;
	strcpy(hostname, "spock.ee.iastate.edu");

	opterr = 0;
	while ((i = getopt(argc, argv, "hpft")) != -1)
	{
		switch (i)
		{
		case 'h':
			strcpy(hostname, argv[optind]);
		break;
		case 't':
			// this is a test flag to show how the flags work
			// this will print out the parms
			printf("%s\n", argv[optind]);
		break;
		case 'p':
			// add code for the p flag set
		break;
		case 'f':
			// add code for the f flag set
		break;
		case '?':
		default:
			done = 1;
		break;
		}
		if (done) break;
	}
	strcpy(buf,"hello there\n");

	h_name = gethostbyname(hostname);
	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(2000);
	sock_in.sin_addr.s_addr	= *(u_long *)h_name->h_addr;
	printf("port = %d -- %s\n",ntohs(sock_in.sin_port),inet_ntoa(sock_in.sin_addr));
	// Send request
	sockFD = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(sockFD, &sock_in, sizeof(sock_in)) < 0) {
		perror("connect request");
		(void) close(sockFD);
		exit(1);
	}
	strcpy(buf,"from client");
	if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
	cp = answer;
	if ((n = recv(sockFD, cp, 100, 0)) < 0){
		perror("SendRequest");
		(void) close(sockFD);
	}
	cp[n] = 0;
	printf("===<%s>===\n",cp);
	(void) close(sockFD);
	exit(1);
}
