/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/stdarg.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <shell/cmd.h>


/* local/static prototypes */
static int help(char const *prog_name, char const *msg);


/* local functions */
static int exec(int argc, char **argv){
	int fd;
	uint8_t slave;


	/* check options */
	if(argc != 3)
		return help(argv[0], 0x0);

	slave = atoi(argv[2]);

	if(slave & 0x80)
		return help(argv[0], "invalid slave address");

	/* apply configuration */
	fd = open(argv[1], O_RDWR);

	if(fd < 0){
		fprintf(stderr, "can't open device %s \"%s\"\n", argv[1], strerror(errno));
		return 1;
	}

	if(ioctl(fd, IOCTL_CFGWR, &slave, 1) != 0){
		fprintf(stderr, "ioctl error \"%s\"\n", strerror(errno));
		return 1;
	}

	return 0;
}

command("i2ccfg", exec);

static int help(char const *prog_name, char const *msg){
	if(msg){
		fprintf(stderr, msg);
		fputs("\n", stderr);
	}

	fprintf(stdout,
		"usage: %s <device> <slave>\n\n"
		"%15.15s    %s\n"
		"%15.15s    %s\n"
		, prog_name
		, "<slave>", "7 bit slave address"
	);

	return (msg ? 1 : 0);

}
