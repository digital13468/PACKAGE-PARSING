all:	udp_server udp_client tcp_server tcp_client

CFLAGS = -g

udp_server:	udp_server.c
	cc udp_server.c ${CFLAGS} -o udp_server 

udp_client:	udp_client.c
	cc udp_client.c ${CFLAGS} -o udp_client 

tcp_client:	tcp_client.c
	cc tcp_client.c ${CFLAGS} -o tcp_client 

tcp_server:	tcp_server.c
	cc tcp_server.c ${CFLAGS} -o tcp_server
