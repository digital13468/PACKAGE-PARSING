/*
Cpr E 530 Program 4
Chan-Ching Hsu

Observation:

tcp_client-
This application include the libarary, "socket.h" to perfrom a sender with tcp data.
It set up with a initial host to link to, spock.ee.iastate.edu, and a initial port, port 2000.
Two variable, buf and anser, are used for buffering the transmiting data to the server and the receiving the data from server, respectively.
The variable hostname accepts a user-defined server name.
By accepting the the flag of p as the variable, port_num, a client can use the particulat port to commincate with the server.
The application will translate the hostname into its correspondin IP address.
The client sends to the server a message, "from client," which will be screened on the server side, 
and then the server would reply with a message, "hello."
By choosing a port as a destination port on server, the client can TELNET the server with that specific prot; 
connection and communication are done as previous situation.
Client also can import a file as a parameter and send the file to the tcp_server application.

tcp_server-
Once the connetion between the client and server has been built, the server can accept and print data to screen infinitely 
until each of both choose to terminate the connection.

udp_client-
It will retry and wait for timeout if the server cannot be recheivable or has no response.
This application also accept and print data to screen indefinitely.

udp_server-
Once connection is built, the server catches the message from its client and sned back a meassage as a greeting.
*/
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
	int			file=0;
	FILE *fp;
	//string filename;
	int 		numTimeOuts	= 0;
	int			port_num = 2000;
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
			port_num = atoi(argv[optind]);
		break;
		case 'f':
			// add code for the f flag set
			//printf("open file\n");
			file=1;
			//filename=argv[optind];
			fp = fopen (argv[optind],"r");
			//fp = fopen ("test.txt","r");
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
	sock_in.sin_port = htons(port_num);
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
	//
	if (file==1){
	while (!feof(fp)){
		
		//fgets(buf,strlen(buf),fp);
		fscanf(fp,"%[^\n] ", &buf);
		//fgetc(fp);
		
		
		send(sockFD, buf, strlen(buf), 0);
		//printf("line 1: %s\n",buf);
		//fscanf(fp," %[a-zA-Z0-9@&;:,. /!?-]\n",buf);
		//printf("line 2: %s",buf);
		//send(sockFD, buf, strlen(buf), 0);
		//fflush(buf);
	}
	fclose(fp);
	}
	//
	while (1){
		scanf("%[^\n]", buf);
		//printf("%s",buf);
		send(sockFD, buf, strlen(buf),0);
		getchar();
		//(void) close(sockFD);
	/*if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}*/
	}
	//
	
	(void) close(sockFD);
	exit(1);
}
