#define RETSIGTYPE void
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef setsignal_h
#define setsignal_h

RETSIGTYPE (*setsignal(int, RETSIGTYPE (*)(int)))(int);
#endif

char cpre580f98[] = "netdump";

void raw_print(u_char *user, const struct pcap_pkthdr *h, const u_char *p);

int packettype;

char *program_name;

unsigned short counter_broadcast, count_IP, count_ARP, count_ICMP, count_TCP, count_DNS;

/* Externs */
extern void bpf_dump(struct bpf_program *, int);

extern char *copy_argv(char **);

/* Forwards */
 void program_ending(int);

/* Length of saved portion of packet. */
int snaplen = 1500;;

static pcap_t *pd;

extern int optind;
extern int opterr;
extern char *optarg;
int pflag = 0, aflag = 0;

int
main(int argc, char **argv)
{
	int cnt, op, i, done = 0;
	bpf_u_int32 localnet, netmask;
	char *cp, *cmdbuf, *device;
	struct bpf_program fcode;
	 void (*oldhandler)(int);
	u_char *pcap_userdata;
	char ebuf[PCAP_ERRBUF_SIZE];

	cnt = -1;
	device = NULL;
	
	if ((cp = strrchr(argv[0], '/')) != NULL)
		program_name = cp + 1;
	else
		program_name = argv[0];

	opterr = 0;
	while ((i = getopt(argc, argv, "pa")) != -1)
	{
		switch (i)
		{
		case 'p':
			pflag = 1;
		break;
		case 'a':
			aflag = 1;
		break;
		case '?':
		default:
			done = 1;
		break;
		}
		if (done) break;
	}
	if (argc > (optind)) cmdbuf = copy_argv(&argv[optind]);
		else cmdbuf = "";

	if (device == NULL) {
		device = pcap_lookupdev(ebuf);
		if (device == NULL)
			error("%s", ebuf);
	}
	pd = pcap_open_live(device, snaplen,  1, 1000, ebuf);
	if (pd == NULL)
		error("%s", ebuf);
	i = pcap_snapshot(pd);
	if (snaplen < i) {
		warning("snaplen raised from %d to %d", snaplen, i);
		snaplen = i;
	}
	if (pcap_lookupnet(device, &localnet, &netmask, ebuf) < 0) {
		localnet = 0;
		netmask = 0;
		warning("%s", ebuf);
	}
	/*
	 * Let user own process after socket has been opened.
	 */
	setuid(getuid());

	if (pcap_compile(pd, &fcode, cmdbuf, 1, netmask) < 0)
		error("%s", pcap_geterr(pd));
	
	(void)setsignal(SIGTERM, program_ending);
	(void)setsignal(SIGINT, program_ending);
	/* Cooperate with nohup(1) */
	if ((oldhandler = setsignal(SIGHUP, program_ending)) != SIG_DFL)
		(void)setsignal(SIGHUP, oldhandler);

	if (pcap_setfilter(pd, &fcode) < 0)
		error("%s", pcap_geterr(pd));
	pcap_userdata = 0;
	(void)fprintf(stderr, "%s: listening on %s\n", program_name, device);
	if (pcap_loop(pd, cnt, raw_print, pcap_userdata) < 0) {
		(void)fprintf(stderr, "%s: pcap_loop: %s\n",
		    program_name, pcap_geterr(pd));
		exit(1);
	}
	pcap_close(pd);
	exit(0);
}

/* routine is executed on exit */
void program_ending(int signo)
{
	struct pcap_stat stat;
	
	extern unsigned short counter_broadcast, count_IP, count_ARP;
	
	if (pd != NULL && pcap_file(pd) == NULL) {
		(void)fflush(stdout);
		putc('\n', stderr);
		if (pcap_stats(pd, &stat) < 0)
			(void)fprintf(stderr, "pcap_stats: %s\n",
			    pcap_geterr(pd));
		else {
			(void)fprintf(stderr, "%d packets received by filter\n",
			    stat.ps_recv);
			(void)fprintf(stderr, "%d packets dropped by kernel\n",
			    stat.ps_drop);
				
			printf("\n%d packets are broadcast packets.\n", counter_broadcast);
			printf("%d packets are IP packets.\n", count_IP);
			printf("%d packets are ARP packets.\n", count_ARP);
			printf("%d packets are ICMP packets.\n", count_ICMP);
			printf("%d packets are TCP packets.\n", count_TCP);
			printf("%d packets are DNS packets.\n", count_DNS);
			printf("\n%d packets received by filter\n",
			    stat.ps_recv);
			printf("%d packets dropped by kernel\n",
			    stat.ps_drop);
		}
	}
	exit(0);
}

/* Like default_print() but data need not be aligned */
void
default_print_unaligned(register const u_char *cp, register u_int length)
{
	register u_int i, s;
	register int nshorts;

	nshorts = (u_int) length / sizeof(u_short);
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0)
			(void)printf("\n\t\t\t");
		s = *cp++;
		(void)printf(" %02x%02x", s, *cp++);
	}
	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("\n\t\t\t");
		(void)printf(" %02x", *cp);
	}
}

/*
 * By default, print the packet out in hex.
 */
void
default_print(register const u_char *bp, register u_int length)
{
	register const u_short *sp;
	register u_int i;
	register int nshorts;

	if ((long)bp & 1) {
		default_print_unaligned(bp, length);
		return;
	}
	sp = (u_short *)bp;
	nshorts = (u_int) length / sizeof(u_short);
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0)
			(void)printf("\n\t");
		(void)printf(" %04x", ntohs(*sp++));
	}
	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("\n\t");
		(void)printf(" %02x", *(u_char *)sp);
	}
}

/*
insert your code in this routine

*/

void raw_print(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
        u_int length = h->len;
        u_int caplen = h->caplen;
		
		unsigned short typefield;
		typefield = p[12]*256+p[13];	//change the type of typefield from hex to dec
		
		int i;
		unsigned short ARP_HT, ARP_PT, ARP_HL, ARP_PL, ARP_Operation, ARP_SHW, APR_SIP, ARP_THW, ARP_TIP;	//for ARP
		unsigned short IP_Pro, IP_Checksum, IP_SIP, IP_DIP, ICMP_TYPE, ICMP_code;	//for IP
		unsigned short TCP_SP, TCP_DP, TCP_SN, TCP_AN, TCP_HL, TCP_Flags, TCP_WS, TCP_Checksum, TCP_UP;	//for TCP
		
		//unsigned short counter_broadcast, count_IP, count_ARP;
		int broadcast1, broadcast2, broadcast3;
		
        default_print(p, caplen);
        putchar('\n');
		
		printf("\nDA = %02x:%02x:%02x:%02x:%02x:%02x, ",p[0],p[1],p[2],p[3],p[4],p[5]);	//print out the destination address
		printf("SA = %02x:%02x:%02x:%02x:%02x:%02x\n",p[6],p[7],p[8],p[9],p[10],p[11]);	//print out the source address
		
		broadcast1 = p[0]*256+p[1];
		broadcast2 = p[2]*256+p[3];
		broadcast3 = p[4]*256+p[5];
		
		if(broadcast3&&broadcast2&&broadcast1==65535){
			//printf("yes\n");
			counter_broadcast=counter_broadcast+1;
		}
		if(typefield >= 1536){	
			printf("Type = 0x%x%02x\n",p[12],p[13]);	//typefield is stating the type

			if(typefield == 2048){	//payload is ip
				printf("Paylaod = IP\n");
				count_IP++;
				
				printf("The IP protocol is version %d.\n",p[14]>>4);
				printf("The length of the IP header is %d bytes.\n",(p[14]&0x0f)*4);
				printf("The length of the payload is %d bytes.\n",p[16]*256+p[17]);
				printf("The identifier is %d.\n",p[18]*256+p[19]);
				printf("Flags are %d %d %d.\n",p[20]>>7,(p[20]>>6)&0x1,(p[20]>>5)&0x1);
				printf("Offset is %d.\n",(p[20]&0x1f)*256+p[21]);
				printf("TTL is %d.\n",p[22]);
				IP_Pro=p[23];
				if(IP_Pro==6){
					count_TCP++;
					printf("The upper-layer protocol handling this packet is TCP.\n");
					printf("Source port number: %d\n", p[34]*256+p[35]);
					printf("Destination port number: %d\n", p[36]*256+p[37]);
					printf("Sequence number: %d\n", p[38]*65536+p[39]*4096+p[40]*256+p[41]);
					printf("Acknowledgement number: %d\n", p[42]*65536+p[43]*4096+p[44]*256+p[45]);
					printf("Length of the TCP header: %d bytes\n", (p[46]>>4)*4);
					TCP_HL=(p[46]>>4)*4;
					if(TCP_HL>20){
						for (i=TCP_HL-19;i<TCP_HL+1;i++){
						//if(p[i]!=0)
						printf("%02x\t",p[i]);
						}	
						printf("\n");
					}
					if (((p[47]>>5)&0x1)==1)
						printf("Packet Contains Urgent Data\n");
					if (((p[47]>>4)&0x1)==1)
						printf("Acknowledgement Number is Valid\n");
					if (((p[47]>>3)&0x1)==1)
						printf("Data Should be Pushed to the Application\n");
					if (((p[47]>>2)&0x1)==1)
						printf("Reset Packet\n");
					if (((p[47]>>1)&0x1)==1)
						printf("Sychronize Packet\n");
					if (((p[47]>>0)&0x1)==1)
						printf("Finish Packet\n");
					printf("Window size: %d bytes\n", p[48]*256+p[49]);
					printf("Chechsum computed using part of the IP header plus the TCP header and date: %d\n", p[50]*256+p[51]);
					
				}
				if(IP_Pro==17){
					printf("The upper-layer protocol handling this packet is UDP.\n");
					if((p[34]*256+p[35])==53||(p[36]*256+p[37])==53)
						count_DNS++;
				}	
				if(IP_Pro==1){
					count_ICMP=count_ICMP+1;
					printf("The upper-layer protocol handling this packet is ICMP.\n");
					ICMP_TYPE=p[34];
					ICMP_code=p[35];
					if(ICMP_TYPE==8)
						printf("ICMP Echo Request\n");
					if(ICMP_TYPE==0)
						printf("ICMP Echo Reply\n");
					if(ICMP_TYPE==3){
						if(ICMP_code==0)
							printf("Network unreachable\n");
						if(ICMP_code==1)
							printf("Host unreachable\n");
						if(ICMP_code==2)
							printf("Protocol unreachable on the target host\n");
						if(ICMP_code==3)
							printf("Port unreachable on the target host\n");
						if(ICMP_code==4)
							printf("Fragmentation needed and DON'T FRAGMENT bit is set\n");
						if(ICMP_code==5)
							printf("Source route failed\n");
					}
					if(ICMP_TYPE==13)
						printf("ICMP Timestamp Request\n");
					if(ICMP_TYPE==14)
						printf("ICMP Timestamp Reply\n");
					if(ICMP_TYPE==11){
						printf("ICMP Time Exceeded\n");
						if(ICMP_code==0)
						printf("TTL count exceeded\n");
						if(ICMP_code==1)
						printf("Fragment reassembly time exceeded\n");
					}
					if(ICMP_TYPE==5){
						printf("ICMP Redirection\n");
						if(ICMP_code==0)
						printf("Network-based redirect\n");
						if(ICMP_code==1)
						printf("Host-based redirect\n");
						if(ICMP_code==2)
						printf("Network-based redirect of the type of service specified\n");
						if(ICMP_code==3)
						printf("Host-based redirect of the type of service specified\n");
					}
					if(ICMP_TYPE==4)
						printf("ICMP Source Quench\n");
					if(ICMP_TYPE==12)
						printf("ICMP Parameter Problem\n");
					if(ICMP_TYPE==15)
						printf("Information Request\n");
					if(ICMP_TYPE==16)
						printf("Information Reply\n");
					if(ICMP_TYPE==17)
						printf("Address Mask Request\n");
					if(ICMP_TYPE==18)
						printf("Address Mask Reply\n");
				}
				printf("The chechsum for IP is %d.\n",p[24]*256+p[25]);
				printf("The IP address of the sender is %d.%d.%d.%d\n",p[26],p[27],p[28],p[29]);
				printf("The IP address of the destination is %d.%d.%d.%d\n",p[30],p[31],p[32],p[33]);
			}	
			if(typefield == 2054){
				printf("Payload = ARP\n");	//payload is arp
				count_ARP = count_ARP+1;
				
				ARP_HT=p[14]*256+p[15];
				if(ARP_HT==1)
					printf("The ARP protocol is used on Ethernet.\n");
				ARP_PT=p[16]*256+p[17];
				if(ARP_PT==2048)
					printf("IP protocol is using the ARP protocol.\n");
				printf("The length of the hardware address fields is %d bytes.\n",ARP_HL=p[18]);
				printf("The length of the upper-layer protocol address is %d bytes.\n",ARP_PL=p[19]);
				ARP_Operation=p[20]*256+p[21];				
				if(ARP_Operation==1)
					printf("This is a request packet.\n");
				if(ARP_Operation==2)
					printf("This is a reply packet.\n");
				printf("Hardware address of the sender-%02x:%02x:%02x:%02x:%02x:%02x\n",p[22],p[23],p[24],p[25],p[26],p[27]);
				printf("IP address of the sender:%d.%d.%d.%d\n",p[28],p[29],p[30],p[31]);
				printf("Hardware address of the target device-%02x:%02x:%02x:%02x:%02x:%02x\n",p[32],p[33],p[34],p[35],p[36],p[37]);
				printf("IP address of the target:%d.%d.%d.%d\n",p[38],p[39],p[40],p[41]);
			}
		}
		if(typefield < 1518)
			printf("Len = %d\n",typefield);	//typefield is describing the length
		
		//printf("%d packets are broadcast packets\n", counter_broadcast);
		//printf("%d packets are IP packets\n", count_IP);
		//printf("%d packets are ARP packets\n", count_ARP);

	
}


