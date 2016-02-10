/*
 ***************************************************************************
 *
 *	UDP server --
 *	routine to wait for a client to send a datagram and then reply with
 *	a datagram, uses port 2000 by default
 *
 ***************************************************************************
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PORT	2000

struct	sockaddr_in nsaddr, rcvaddr;
struct	sockaddr_in from_addr;		/* Source addr of last packet */
int	from_len;			/* Source addr size of last packet */

extern int errno, opterr;

main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	int n, vs;
	int i, blen, addlen, done=0;
	char buf[4048];

	rcvaddr.sin_family = AF_INET;
	nsaddr.sin_family = AF_INET;
	nsaddr.sin_addr.s_addr = INADDR_ANY;
	nsaddr.sin_port = htons(PORT);

	opterr = 0;
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
	/*
	** Open stream port.
	*/
	if ((vs = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("SERVER: socket(SOCK_DGRAM): %d\n",errno);
		exit(1);
	}	
	if (bind(vs, (struct sockaddr *) &nsaddr, sizeof(nsaddr)) < 0) {
		printf("bind(vs, %s[%d]) errno = %d\n "
		 ,inet_ntoa(nsaddr.sin_addr), ntohs(nsaddr.sin_port),errno);
		exit(1);
	}
	strcpy(buf, inet_ntoa(nsaddr.sin_addr));
	printf("SERVER: bind(vs, %s:", inet_ntoa(nsaddr.sin_addr));
	printf("[%d]):\n ", ntohs(nsaddr.sin_port));
	printf("SERVER: waiting  buf size = %d\n",sizeof(buf));
	addlen = sizeof(rcvaddr);
	blen = recvfrom(vs,buf,sizeof(buf), 0 , (struct sockaddr *) &rcvaddr, &addlen);
	printf("SERVER: data from (vs, %s[%d]):\n ",
		inet_ntoa(rcvaddr.sin_addr), ntohs(rcvaddr.sin_port));
	buf[blen] = 0;
	printf("--<%s>--",buf);
	strcpy(buf,"hello from server\n");
	printf("SERVER: sending\n");
	if (sendto(vs, buf, strlen(buf), 0, (struct sockaddr *) &rcvaddr,
 	    sizeof(rcvaddr)) != strlen(buf)) {
		    perror("Sendto");
	}
	sleep(5);
}
