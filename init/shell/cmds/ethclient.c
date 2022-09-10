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
#define ARGS	"<udp|tcp> <ip> <port>"
#define OPTS \
	"-t <timeout>", "time to wait for incoming data [ms]", \
	"-h", "print this help message"

#define ERROR(fmt, ...)({ fprintf(stderr, "error " fmt, ##__VA_ARGS__); -1; })


/* types */
typedef struct{
	sock_type_t type;
	inet_addr_t ip;
	uint16_t port;
	size_t rx_timeout_ms;
} opts_t;


/* local/static prototypes */
static int client(int sock);
static int rx_loop(int sock);
static int parse_opts(int argc, char **argv);


/* static variables */
static opts_t opts = {
	.rx_timeout_ms = 1000,
};


/* local functions */
static int exec(int argc, char **argv){
	int r;
	int sock;


	/* parse command line options */
	if((r = parse_opts(argc, argv)) != 0)
		return r > 0;

	/* start server */
	printf("connect %s client to %s:%d\n",
		(opts.type == SOCK_STREAM ? "tcp" : "udp"),
		inet_ntoa(opts.ip),
		opts.port
	);

	sock = socket(AF_INET, opts.type);

	if(sock < 0)
		return ERROR("socket(): %s\n", strerror(errno));

	(void)client(sock);

	if(close(sock) != 0)
		return ERROR("close(): %s\n", strerror(errno));

	return 0;
}

command("ethclient", exec);


static int client(int sock){
	char c;
	char line[16];
	ssize_t i;
	sock_addr_inet_t addr;
	f_mode_t f_mode;


	if(fcntl(sock, F_MODE_GET, &f_mode, sizeof(f_mode_t)) != 0)
		return ERROR("fcntl(get mode): %s\n", strerror(errno));

	f_mode |= O_NONBLOCK;

	if(fcntl(sock, F_MODE_SET, &f_mode, sizeof(f_mode_t)) != 0)
		return ERROR("fcntl(set mode): %s\n", strerror(errno));

	addr.domain = AF_INET;
	addr.data.addr = opts.ip;
	addr.data.port = opts.port;

	if(connect(sock, (sock_addr_t*)&addr, sizeof(sock_addr_inet_t)) != 0)
		return ERROR("connect(): %s\n", strerror(errno));

	while(1){
		write(1, "cl$ ", 4);

		i = 0;
		c = 0;

		while(i < 15 && c != '\n'){
			if(read(0, &c, 1) != 1)
				return ERROR("read(stdin): %s\n", strerror(errno));

			if(c == '\r')
				continue;

			write(1, &c, 1);
			line[i++] = c;
		}

		line[i - 1] = 0;

		if(send(sock, line, i) != i)
			return ERROR("send(): %s\n", strerror(errno));

		(void)rx_loop(sock);

		if(strcmp(line, "q") == 0)
			break;
	}

	return 0;
}

static int rx_loop(int sock){
	int r;
	unsigned int attempts;
	size_t sleep_ms;
	char data[8];


	attempts = 0;
	sleep_ms = opts.rx_timeout_ms / 4;

	while(1){
		r = recv(sock, data, 7);

		if(r < 0)
			return ERROR("recv(): %s\n", strerror(errno));

		if(attempts > 5)
			return 0;

		if(r == 0){
			attempts++;
			sleep(sleep_ms, 0);
			continue;
		}

		attempts = 0;
		data[r] = 0;
		printf("recv: %s\n", data);
	}
}

static int parse_opts(int argc, char **argv){
	char opt;


	while((opt = getopt(argc, argv, "t:h")) != -1){
		switch(opt){
		case 't':	opts.rx_timeout_ms = atoi(optarg); break;
		case 'h':	CMD_HELP(argv[0], 0x0); return -1;
		default:	return CMD_HELP(argv[0], "");
		};
	}

	if(optind + 2 >= argc)
		return CMD_HELP(argv[0], "missing argument");

	if(strcmp(argv[optind], "udp") == 0)		opts.type = SOCK_DGRAM;
	else if(strcmp(argv[optind], "tcp") == 0)	opts.type = SOCK_STREAM;
	else										return CMD_HELP(argv[0], "invalid type");

	opts.ip = inet_addr(argv[optind + 1]);
	opts.port = atoi(argv[optind + 2]);

	return 0;
}
