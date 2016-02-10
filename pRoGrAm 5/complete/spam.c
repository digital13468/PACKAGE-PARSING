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
	char		buf[100],answer[4048], user[100],target[100],path[100];
	struct	hostent	*h_name;
	struct	servent	*s_name;

	int 		numTimeOuts	= 0;
	int 		file =0;
	
	//setup default values
	strcpy(user, "fake@iastate.edu");
	strcpy(target, "beinhart@spock.ee.iastate.edu");		

	opterr = 0;
	while ((i = getopt(argc, argv, "sudf")) != -1)
	{
		switch (i)
		{
		case 'u':
			// code for s flag
			strcpy(target, argv[optind]);
			optind++;
			
		break;
		case 's':
			// code for s flag
			strcpy(user, argv[optind]);
			optind++;
		break;
		case 'd':
			debug = 1;
		break;
		case 'f':
			// code for u flag
			strcpy(path, argv[optind]);
			file =1;
			optind++;
		break;
		case '?':
		default:
			done = 1;
		break;
		}
		if (done) break;
	}
	sockFD = -1;
	printf("\n\nSent from address:[%s]\n", user);
	printf("Target address:[%s]\n\n", target);
	h_name = gethostbyname("spock.ee.iastate.edu");
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

	strcpy(buf,"helo bones.ee.iastate.edu\r\n");
	
	printf("[%s]\n", answer);
	if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
	if (getline(sockFD, answer) == 0){
		printf("error: no response from server\n");
		exit(0);
	}
	
	if (answer[0] != '2'|| answer[1] != '5'||answer[2] != '0') {
		printf("error: Incorrect response from server\n");
		(void) close(sockFD);
		exit(1);
	}
	printf("[%s]\n", answer);
	
	//Send mail from
	strcpy(buf,"mail from: ");
	strcat(buf,user);
	strcat(buf,"\r\n");
	printf("[%s]\n", buf);
	if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
	if (getline(sockFD, answer) == 0){
		printf("error: no response from server\n");
		exit(0);
	}
	if (answer[0] != '2'|| answer[1] != '5'||answer[2] != '0') {
		printf("error: Incorrect response from server Check sender address\n Program Terminateing");
		(void) close(sockFD);
		exit(1);
	}
	printf("[%s]\n", answer);
	//Send rcpt to
	strcpy(buf,"rcpt to: ");
	strcat(buf,target);
	strcat(buf,"\r\n");
	
	if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
	if (getline(sockFD, answer) == 0){
		printf("error: no response from server\n");
		exit(0);
	}
	if (answer[0] != '2'|| answer[1] != '5'||answer[2] != '0') {
		printf("error: Incorrect response from server Check Target\n Program Terminating");
		(void) close(sockFD);
		exit(1);
	}
	printf("[%s]\n", answer);
	//send data
	strcpy(buf,"data\r\n");
	
	
	if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
	if (getline(sockFD, answer) == 0){
		printf("error: no response from server\n");
		exit(0);
	}
	if (answer[0] != '3'|| answer[1] != '5'||answer[2] != '4') {
		printf("error: Incorrect response from server\n");
		(void) close(sockFD);
		exit(1);
	}
	printf("[%s]\n", answer);
	
	//send message
	if(!file)
	{
		strcpy(buf,"youve been spammed\r\n");
		if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
	}
	else{
		FILE * fd = fopen(path,"rb");
		if(fd==NULL)
		{
			printf("FIle Not Found: Program Terminating\n");
			exit(0);
		}
		else{
			printf("File Found\n");
		}
		printf("Sending file\n");
		long fileSize=0;
		fseek(fd,0,SEEK_END);
		fileSize=ftell(fd);
		rewind(fd);
		long read=0;
		int used=0;
		printf("Filesize: %d Bytes\n",fileSize);
		while(read<fileSize)	// not really important since its captured by the EOF
		{
			int i=0;
			used=0;
			for(;i<100;i++)
			{
				buf[i]=fgetc(fd);			// for some reason fread will not work. This can be used in replacement
				//printf("%c",buf[i]);
				if(buf[i]==EOF)
				{
					i=100;
					used--;	// remove the EOF
					read--;
				}
				read++;
				used++;
			}
			
			if (send(sockFD, buf, used,0) != used) {
			perror("send request");
			(void) close(sockFD);
			}
			
		}
	}
	
	

	
	strcpy(buf,".\r\n");
		if (send(sockFD, buf, strlen(buf),0) != strlen(buf)) {
		perror("send request");
		(void) close(sockFD);
		exit(1);
	}
	
	
	if (getline(sockFD, answer) == 0){
		printf("error: no response from server\n");
		exit(0);
	}
	printf("[%s]\n", answer);
	
	
	
	
	
	
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

