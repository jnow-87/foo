/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/stdarg.h>
#include <sys/string.h>
#include <sys/net.h>
#include <sys/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <socket.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<udp|tcp> <port>"
#define OPTS \
	"-e", "send a reply on incoming data", \
	"-h", "print this help message"

#define ERROR(fmt, ...)({ fprintf(stderr, "error " fmt, ##__VA_ARGS__); -1; })


/* types */
typedef struct{
	sock_type_t type;
	uint16_t port;
	bool echo;
} opts_t;


/* local/static prototypes */
static int server(int sock);
static void rx_loop(int sock);


/* static variables */
static opts_t opts = {
	.echo = false,
};


/* local functions */
static int exec(int argc, char **argv){
	char opt;
	int sock;


	/* parse command line options */
	while((opt = getopt(argc, argv, "eh")) != -1){
		switch(opt){
		case 'e':	opts.echo = true; break;
		case 'h':	return CMD_HELP(argv[0], 0x0);
		default:	return CMD_HELP(argv[0], "");
		};
	}

	if(optind + 1 >= argc)
		return CMD_HELP(argv[0], "missing argument");

	if(strcmp(argv[optind], "udp") == 0)		opts.type = SOCK_DGRAM;
	else if(strcmp(argv[optind], "tcp") == 0)	opts.type = SOCK_STREAM;
	else										return CMD_HELP(argv[0], "invalid type");

	opts.port = atoi(argv[optind + 1]);

	/* start server */
	printf("start %s server on port %d\n",
		(opts.type == SOCK_STREAM ? "tcp" : "udp"),
		opts.port
	);

	sock = socket(AF_INET, opts.type);

	if(sock < 0)
		return ERROR("socket(): %s\n", strerror(errno));

	(void)server(sock);

	if(close(sock) != 0)
		return ERROR("server close(): %s\n", strerror(errno));

	return 0;
}

command("ethserver", exec);


static int server(int sock){
	size_t addr_len;
	sock_addr_inet_t addr;


	addr.domain = AF_INET;
	addr.inet_data.addr = INET_ADDR_ANY;
	addr.inet_data.port = opts.port;

	if(bind(sock, (sock_addr_t*)&addr, sizeof(sock_addr_inet_t)) != 0)
		return ERROR("bind(): %s\n", strerror(errno));

	if(opts.type == SOCK_STREAM && listen(sock, 2) != 0)
		return ERROR("liste(): %s\n", strerror(errno));

	printf("server awaiting connection\n");

	if(opts.type == SOCK_STREAM){
		addr_len = sizeof(sock_addr_inet_t);
		memset(&addr, 0, addr_len);

		sock = accept(sock, (sock_addr_t*)&addr, &addr_len);

		if(sock < 0)
			return ERROR("accept(): %s\n", strerror(errno));

		printf("client: %s on port %u\n", inet_ntoa(addr.inet_data.addr), addr.inet_data.port);
	}

	rx_loop(sock);

	if(opts.type == SOCK_STREAM){
		if(close(sock) != 0)
			return ERROR("client close(): %s\n", strerror(errno));
	}

	return 0;
}

static void rx_loop(int sock){
	int r;
	char s[16];
	char *buf;
	sock_addr_inet_t remote;
	size_t addr_len;


	strcpy(s, "echo: ");
	buf = s + 6;

	printf("rx loop quits if \"q\" is received\n");

	while(1){
		addr_len = sizeof(remote);
		memset(&remote, 0, addr_len);

		r = recvfrom(sock, buf, 9, (sock_addr_t*)&remote, &addr_len);

		if(r < 0){
			printf("error: %d %s\n", r, strerror(errno));
			break;
		}

		buf[r] = 0;

		if(opts.type == SOCK_DGRAM)
			printf("recv %s:%u: %s\n", inet_ntoa(remote.inet_data.addr), remote.inet_data.port, buf);
		else
			printf("recv: %s\n", buf);

		if(strcmp(buf, "q") == 0)
			break;

		if(opts.echo){
			r = strlen(s);

			if(sendto(sock, s, strlen(s), (sock_addr_t*)&remote, addr_len) != r)
				printf("send error: %s\n", strerror(errno));
		}
	}
}
