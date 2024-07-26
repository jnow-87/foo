/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/i2c.h>
#include <sys/ioctl.h>
#include <sys/stdarg.h>
#include <sys/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <shell/shell.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<device> master <7-bit slave> | <device> slave"
#define OPTS	""


/* local functions */
static int exec(int argc, char **argv){
	int fd;
	char *dev;
	i2c_dev_cfg_t cfg;


	/* check options */
	if(argc < 3)
		return CMD_HELP(argv[0], 0x0);

	dev = argv[1];
	cfg.mode = (strcmp(argv[2], "master") == 0) ? I2C_MASTER : I2C_SLAVE;

	if(cfg.mode == I2C_MASTER){
		if(argc != 4)
			return CMD_HELP(argv[0], 0x0);

		cfg.slave = atoi(argv[3]);

		if(cfg.slave & 0x80)
			return CMD_HELP(argv[0], "invalid slave address");
	}

	/* apply configuration */
	fd = open(dev, O_RDWR);

	if(fd < 0)
		return -ERROR("opening %s", dev);

	if(ioctl(fd, IOCTL_CFGWR, &cfg) != 0)
		return -ERROR("ioctl");

	return 0;
}

command("i2ccfg", exec);
