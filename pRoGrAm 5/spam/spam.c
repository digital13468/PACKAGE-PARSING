/*
 ***********************************************************************
 *
 *  spam.c
 *	Routine to send spam mail to a machine
 *
***********************************************************************
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>


/*
 *  Initialize the socket address info struct.
 */

struct sockaddr_in s_in, temp, from_addr;
int	from_len;

extern	int	errno;
int debug = 0;

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

	int 		numTimeOuts	= 0;


	opterr = 0;
	while ((i = getopt(argc, argv, "sud")) != -1)
	{
		switch (i)
		{
		case 'u':
			// code for s flag
		break;
		case 's':
			// code for u flag
		break;
		case 'd':
			debug = 1;
		break;
		case '?':
		default:
			done = 1;
		break;
		}
		if (done) break;
	}
	sockFD = -1;

	h_name = gethostbyname("scotty.ee.iastate.edu");
	s_name = getservbyname("smtp", "tcp");
	s_in.sin_port	= s_name->s_port; 
	s_in.sin_family = AF_INET;
	s_in.sin_addr.s_addr	= *(u_long *)h_name->h_addr;
	printf("port = %d -- %s\n",ntohs(s_in.sin_port),inet_ntoa(s_in.sin_addr));
	/*
	 * Send request
	 */
	sockFD = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(sockFD, (struct sockaddr * )&s_in, sizeof(s_in)) < 0) {
		perror("connect request");
		(void) close(sockFD);
		exit(1);
	}
	if (getline(sockFD, answer) == 0){
		printf("error: no response from server\n");
		exit(0);
	}
	if (answer[0] == '2') printf("got OK from server\n");
// add your code here
	printf("[%s]\n", answer);
	if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
// add your code above here
	(void) close(sockFD);
	exit(1);
}

int getline(int fd, char *answer)
{
	char *cp = answer;
        struct timeval  timeout;
	int dsmask, reply, done = 0;
	char buf[100];
	int n = 0;
	answer[0] = 0;
        /* Wait for reply */
	while (1)
	{
        	timeout.tv_sec = 4;
        	timeout.tv_usec = 0;
        	dsmask = 1 << fd;
        	n = select(fd+1, &dsmask, 0, 0, &timeout);
        	if (n < 0) {
                	perror("spam select error");
                	return 0;
        	}
        	if (n == 0) {    /* timeout */
                	printf("spam: mask = %d after slect call\n",dsmask);
                	return 0;
        	}
		if ((n = recv(fd, buf, 100, 0)) < 0){
			perror("recv");
			(void) close(fd);
			return 0;
		}
		if ((buf[n-1] == '\n') || (buf[n-1] == '\r')) done = 1;
		buf[n-1]=0;
		if (debug) printf("<%s>\n", buf);
		strcat(answer, buf);
		if (done) return 1;
	}
}

