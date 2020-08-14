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
#include <socket.h>
#include <shell/cmd.h>


/* macros */
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
static int parse_opts(int argc, char **argv);
static int help(char const *pname, char const *fmt, ...);


/* static variables */
static opts_t opts = {
	.echo = false,
};


/* local functions */
static int exec(int argc, char **argv){
	int sock;


	/* parse command line options */
	if(parse_opts(argc, argv) != 0)
		return 1;

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
	addr.data.addr = INET_ADDR_ANY;
	addr.data.port = opts.port;

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

		printf("client: %s on port %u\n", inet_ntoa(addr.data.addr), addr.data.port);
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
	char *data;
	sock_addr_inet_t remote;
	size_t addr_len;


	strcpy(s, "echo: ");
	data = s + 6;

	printf("rx loop quits if \"q\" is received\n");

	while(1){
		addr_len = sizeof(remote);
		memset(&remote, 0, addr_len);

		r = recvfrom(sock, data, 9, (sock_addr_t*)&remote, &addr_len);

		if(r < 0){
			printf("error: %d %s\n", r, strerror(errno));
			break;
		}

		data[r] = 0;

		if(opts.type == SOCK_DGRAM)
			printf("recv %s:%u: %s\n", inet_ntoa(remote.data.addr), remote.data.port, data);
		else
			printf("recv: %s\n", data);

		if(strcmp(data, "q") == 0)
			break;

		if(opts.echo){
			r = strlen(s);

			if(sendto(sock, s, strlen(s), (sock_addr_t*)&remote, addr_len) != r)
				printf("send error: %s\n", strerror(errno));
		}
	}
}

static int parse_opts(int argc, char **argv){
	int i;


	for(i=1; i<argc && argv[i][0]=='-'; i++){
		switch(argv[i][1]){
		case 'e':
			opts.echo = true;
			break;

		default:
			return help(argv[0], "invalid option \"%s\"", argv[i]);
		};
	}

	if(i + 2 > argc)
		return help(argv[0], "missing argument");

	if(strcmp(argv[i], "udp") == 0)			opts.type = SOCK_DGRAM;
	else if(strcmp(argv[i], "tcp") == 0)	opts.type = SOCK_STREAM;
	else									return help(argv[0], "invalid type");

	opts.port = atoi(argv[i + 1]);

	return 0;
}

static int help(char const *pname, char const *fmt, ...){
	va_list lst;


	if(fmt){
		va_start(lst, fmt);
		vfprintf(stderr, fmt, lst);
		va_end(lst);
		fputs("\n\n", stderr);
	}

	fprintf(stderr,
		"usage: %s [<options>] <udp|tcp> <port>\n"
		"\noptions:\n"
		"%15.15s    %s\n"
		, pname
		, "-e", "send a reply on incoming data"
	);

	return (fmt ? 1 : 0);
}
