/*
 *******************************************************************************
 *
 *  client.c --
 *	Routine to send datagram packets to a server 
 *	default port is 2000 and default server is spock
 *
 *******************************************************************************
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define	HOST	"spock.ee.iastate.edu"
#define PORT	2000

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
	struct	hostent	*h_name;
	struct	servent	*s_name;
	char	hostname[100];

	int 		numTimeOuts	= 0;

	sockFD = -1;
	
	opterr = 0;
	strcpy(hostname, HOST);
	while ((i = getopt(argc, argv, "pft")) != -1)
	{
		switch (i)
		{
		case 't':
			// this is a test flag to show how the flags work
			// this will print out the parms
			printf("%s\n", argv[optind]);
		break;
		case 'p':
			// add code for the p flag set
		break;
		case 'h':
			// copy parm to host name
			strcpy(hostname, argv[optind]);
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
	/*
	 * Send request, RETRY times, or until successful
	 */
	for (retry = 4; --retry >= 0; ) {
		if (sockFD < 0) {
			sockFD = socket(AF_INET, SOCK_DGRAM, 0);
			if (sockFD < 0) perror("CLIENT: SendRequest1");
			temp.sin_family = AF_INET;
			temp.sin_port = htons(0);
			temp.sin_addr.s_addr = INADDR_ANY;
			if (bind(sockFD,(struct sockaddr *)&temp,sizeof(temp)) < 0)
				printf("bind error errno = %d\n",errno);
		}
		printf("send message\n");
		if (sendto(sockFD, buf, strlen(buf), 0, (struct sockaddr *) &sock_in,
 		    sizeof(sock_in)) != strlen(buf)) {
			    perror("CLIENT: Sendto");
		}
		/* Wait for reply */
		timeout.tv_sec = 4;
		printf("timeout = %d\n",timeout.tv_sec);
		timeout.tv_usec = 0;
		dsmask = 1 << sockFD;
		printf("CLIENT: mask = %d sockFD = %d\n",dsmask,sockFD);
		n = select(sockFD+1, &dsmask, 0, 0, &timeout);
		if (n < 0) {
			perror("CLIENT: select error");
			continue;
		}
		if (n == 0) {	 /* timeout */
			printf("CLIENT: mask = %d after slect call\n",dsmask);
			printf("CLIENT: Timeout %d\n", ++numTimeOuts);
			continue;
		}
		printf("CLIENT: mask = %d sockFD = %d after select call\n",dsmask, sockFD);
 		if ((resplen = recv(sockFD, answer, sizeof(answer), 0)) <= 0) {
			printf("CLIENT: errno = %d resplen = %d\n",errno,resplen);
			printf("CLIENT: fromlen = %d\n",from_len);
			perror("CLIENT: recvfrom error");
			continue;
		}
		buf[resplen] = 0;
		printf("CLIENT: Got answer (%d bytes):\n", resplen);
		printf("CLIENT: ==<%s>==",buf);
		(void) close(sockFD);
		sockFD = -1;
		exit(0);
	}
	(void) close(sockFD);
	sockFD = -1;
	exit(1);
}
