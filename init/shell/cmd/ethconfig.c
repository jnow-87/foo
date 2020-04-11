/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/stdarg.h>
#include <sys/net.h>
#include <sys/inet.h>
#include <sys/string.h>
#include <unistd.h>
#include <stdio.h>
#include <shell/cmd.h>


/* macros */
#define OPTS_STRCPY(opts, attr, val){ \
	if(sizeof((opts).attr) <= strlen(val)) \
		return help(argv[0], "%s too long, max. %d", #attr, sizeof((opts).attr)); \
	\
	strcpy((opts).attr, val); \
}


/* types */
typedef struct{
	char const *dev;
	inetdev_cfg_t dev_cfg;
} opts_t;


/* local/static prototypes */
static int parse_opts(int argc, char **argv);
static int help(char const *pname, char const *fmt, ...);


/* static variables */
static opts_t opts = {
	.dev = "/dev/eth0",
	.dev_cfg = {
		.enc = ENC_WPA2_PSK,
		.dhcp = true,
		.mode = INET_CLIENT,
		.hostname = "bricknode",
	},
};


/* local functions */
static int exec(int argc, char **argv){
	int fd;
	int r;


	/* parse command line options */
	if(parse_opts(argc, argv) != 0)
		return 1;

	/* printf configuation */
	printf("applying following configuration to %s\n", opts.dev);
	printf("    mode: %s\n", (opts.dev_cfg.mode == INET_AP ? "AP" : "CLIENT"));
	printf("    ssid: %s\n", opts.dev_cfg.ssid);
	printf("    password: %s\n", opts.dev_cfg.password);
	printf("    hostname: %s\n", opts.dev_cfg.hostname);

	if(!opts.dev_cfg.dhcp){
		printf("ip: %s\n", inet_ntoa(opts.dev_cfg.ip));
		printf("gateway: %s\n", inet_ntoa(opts.dev_cfg.gateway));
		printf("netmask: %s\n", inet_ntoa(opts.dev_cfg.netmask));
	}
	else
		printf("    dhcp: on\n");

	printf("\n");

	/* apply configuration */
	fd = open(opts.dev, O_RDWR);

	if(fd < 0){
		fprintf(stderr, "error opening device \"%s\"\n", strerror(errno));
		return 1;
	}

	r = 0;
	r |= ioctl(fd, IOCTL_CFGWR, &opts.dev_cfg, sizeof(opts.dev_cfg));
	r |= ioctl(fd, IOCTL_CFGRD, &opts.dev_cfg, sizeof(opts.dev_cfg));

	if(r != 0)
		fprintf(stderr, "error configuring device \"%s\"\n", strerror(errno));

	if(r == 0 && opts.dev_cfg.dhcp){
		printf("\ndhcp configuration\n");
		printf("    ip: %s\n", inet_ntoa(opts.dev_cfg.ip));
		printf("    gateway: %s\n", inet_ntoa(opts.dev_cfg.gateway));
		printf("    netmask: %s\n", inet_ntoa(opts.dev_cfg.netmask));
	}

	close(fd);

	return r != 0;
}

command("ethconfig", exec);


static int parse_opts(int argc, char **argv){
	int i;


	for(i=1; i<argc && argv[i][0]=='-'; i++){
		switch(argv[i][1]){
		case 'd':
			if(++i >= argc)
				help(argv[0], "missing argument to -d");

			opts.dev = argv[i];
			break;

		case 'h':
			if(++i >= argc)
				help(argv[0], "missing argument to -h");

			OPTS_STRCPY(opts.dev_cfg, hostname, argv[i]);
			break;

		case 't':
			if(++i >= argc)
				help(argv[0], "missing argument to -t");

			if(strcmp(argv[i], "client") == 0)	opts.dev_cfg.mode = INET_CLIENT;
			else if(strcmp(argv[i], "ap") == 0)	opts.dev_cfg.mode = INET_AP;
			else								return help(argv[0], "invalid mode \"%s\"", argv[i]);
			break;

		case 's':
			if(i + 3 >= argc)
				help(argv[0], "missing argument to -s");

			opts.dev_cfg.dhcp = false;

			opts.dev_cfg.ip = inet_addr(argv[++i]);
			opts.dev_cfg.gateway = inet_addr(argv[++i]);
			opts.dev_cfg.netmask = inet_addr(argv[++i]);
			break;

		default:
			return help(argv[0], "invalid option \"%s\"", argv[i]);
		};
	}

	if(i + 2 > argc)
		return help(argv[0], "missing argument");

	OPTS_STRCPY(opts.dev_cfg, ssid, argv[i]);
	OPTS_STRCPY(opts.dev_cfg, password, argv[i + 1]);

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
		"usage: %s [<options>] <ssid> <password>\n"
		"\noptions:\n"
		"%22.22s    %s\n"
		"%22.22s    %s\n"
		"%22.22s    %s\n"
		, pname
		, "-d <device>", "target net-device"
		, "-t <ap|client>", "set device to access point or client mode"
		, "-s <ip> <gw> <nm>", "use static ip configuration"
	);

	return (fmt ? 1 : 0);
}
