/*
Cpr E 530 Program 4
Chan-Ching Hsu

The observation was stated in tcp_client.c.
*/
/*
 ***************************************************************************
 *
 *	tcp_server --
 *	wait for a client to connect and then exchange packets
 *	uses port 2000 as a default well know address
 *
 ***************************************************************************
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

struct	sockaddr_in nsaddr, rcvaddr;
struct	sockaddr_in from_addr;		/* Source addr of last packet */
int	from_len;			/* Source addr size of last packet */
int loop;
#define PORT	2000

extern int errno;

main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	int vs, ns, blen, done=0, i;
	int port_num = PORT;
	char buf[4048];

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
			port_num = atoi(argv[optind]);
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
	nsaddr.sin_family = AF_INET;
	nsaddr.sin_addr.s_addr = INADDR_ANY;
	nsaddr.sin_port = htons(port_num);
	/*
	** Open stream port.
	*/
	if ((vs = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket(SOCK_DGRAM): %d\n",errno);
		exit(1);
	}	
	if (bind(vs, (struct sockaddr *)&nsaddr, sizeof(nsaddr)) < 0) {
		printf("bind(vs, %s[%d]) errno = %d\n "
		 ,inet_ntoa(nsaddr.sin_addr), ntohs(nsaddr.sin_port),errno);
		perror("bind error");
		exit(1);
	}
	fprintf(stderr,"SERVER: bind(vs, %s[%d]):\n ",
		inet_ntoa(nsaddr.sin_addr), ntohs(nsaddr.sin_port));
	printf("SERVER: listen waiting\n");
	if ((listen(vs,5)) < 0 ) {
		perror("listen");
		exit(1);
	}
	printf("SERVER: waiting  buf size = %d\n",sizeof(buf));
	from_len = sizeof(from_addr);
	if ((ns = accept(vs, (struct sockaddr *) &from_addr, &from_len)) < 0) perror("accept");	
	printf("SERVER: accepted call\n");
	fprintf(stderr,"SERVER: from_addr(ns, %s[%d]):\n ",
		inet_ntoa(from_addr.sin_addr), ntohs(from_addr.sin_port));
	blen = recv(ns,buf,sizeof(buf), 0);
	buf[blen] = 0;
	printf("SERVER: --<%s>--\n",buf);
	strcpy(buf,"hello");
	printf("SERVER: sending\n");
	if (send(ns, buf, strlen(buf), 0) != strlen(buf)) {
		    perror("Sendto");
	}
	//
	blen=recv(ns,buf,sizeof(buf),0);
	buf[blen]=0;
	while (blen>0){
	
	printf("SERVER: --<%s>--\n",buf);
	
	blen=recv(ns,buf,sizeof(buf),0);
	printf("%d",blen);
	buf[blen]=0;
	//if (blen>0)
	//printf("SERVER: --<%s>--\n",buf);
	
	//fflush(buf);
	
	//if(blen==0)
	
	//break;
	}
	//
	//
	
	shutdown(ns,2);
}
