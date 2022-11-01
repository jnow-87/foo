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
#include <getopt.h>
#include <shell/shell.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<ssid> <password>"
#define OPTS \
	"-d <device>", "target net-device", \
	"-t ap|client", "set device to access point or client mode", \
	"-s <ip> <gateway> <netmask>", "use static ip configuration", \
	"-h <host>", "set host name"

#define OPTS_STRCPY(opts, attr, val){ \
	if(sizeof((opts).attr) <= strlen(val)) \
		return ERROR("%s too long, max. %d", #attr, sizeof((opts).attr)); \
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
	int r = 0;
	int fd;


	/* parse command line options */
	if(parse_opts(argc, argv) != 0)
		return 1;

	/* printf configuation */
	printf("configuration for %s\n", opts.dev);
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

	if(fd < 0)
		return -ERROR("opening device");

	r |= ioctl(fd, IOCTL_CFGWR, &opts.dev_cfg);
	r |= ioctl(fd, IOCTL_CFGRD, &opts.dev_cfg);

	if(r != 0)
		ERROR("configure device");

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
	char opt;


	while((opt = getopt(argc, argv, "d:h:t:s")) != -1){
		switch(opt){
		case 'd':	opts.dev = optarg; break;
		case 'h':	OPTS_STRCPY(opts.dev_cfg, hostname, optarg); break;
		case 't':
			if(strcmp(optarg, "client") == 0)	opts.dev_cfg.mode = INET_CLIENT;
			else if(strcmp(optarg, "ap") == 0)	opts.dev_cfg.mode = INET_AP;
			else								return CMD_HELP(argv[0], "invalid mode");
			break;

		case 's':
			if(optind + 2 >= argc)
				return CMD_HELP(argv[0], "missing argument to -s");

			opts.dev_cfg.dhcp = false;

			opts.dev_cfg.ip = inet_addr(argv[optind++]);
			opts.dev_cfg.gateway = inet_addr(argv[optind++]);
			opts.dev_cfg.netmask = inet_addr(argv[optind]);
			break;

		default:
			return CMD_HELP(argv[0], "");
		};
	}

	if(optind + 1 >= argc)
		return CMD_HELP(argv[0], "missing argument");

	OPTS_STRCPY(opts.dev_cfg, ssid, argv[optind]);
	OPTS_STRCPY(opts.dev_cfg, password, argv[optind + 1]);

	return 0;
}
