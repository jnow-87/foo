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
#include <sys/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <shell/cmd.h>


/* local/static prototypes */
static int help(char const *prog_name, char const *msg);


/* local functions */
static int exec(int argc, char **argv){
	int fd;
	i2c_cfg_t cfg;


	memset(&cfg, 0, sizeof(i2c_cfg_t));

	/* check options */
	if(argc < 6)
		return help(argv[0], 0x0);

	cfg.mode = atoi(argv[2]);
	cfg.host_addr = atoi(argv[3]);
	cfg.target_addr = atoi(argv[4]);
	cfg.clock_khz = atoi(argv[5]);

	if(cfg.mode != I2C_MODE_MASTER && cfg.mode != I2C_MODE_SLAVE)
		return help(argv[0], "invalid mode");

	if(!cfg.host_addr || cfg.host_addr & 0x80)
		return help(argv[0], "invalid host address");

	if(!cfg.target_addr || cfg.target_addr & 0x80)
		return help(argv[0], "invalid target address");

	/* apply configuration */
	fd = open(argv[1], O_RDWR);

	if(fd < 0){
		fprintf(stderr, "can't open device %s \"%s\"\n", argv[1], strerror(errno));
		return 1;
	}

	if(ioctl(fd, IOCTL_CFGWR, &cfg, sizeof(i2c_cfg_t)) != 0){
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
		"usage: %s <device> <mode> <host addr> <target addr> <freq [kHz]>\n\n"
		"%15.15s    %s\n"
		"%15.15s    %s\n"
		, prog_name
		, "<mode>", "master (1), slave(2)"
		, "<* addr>", "7 bit address"
	);

	return (msg ? 1 : 0);

}
