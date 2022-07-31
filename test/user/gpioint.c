/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/gpio.h>
#include <sys/fcntl.h>
#include <sys/escape.h>
#include <test/test.h>


/* macros */
#define GPIO_DEV	"/dev/int0"
#define SIGNAL		SIG_USR1


/* local/static prototypes */
static void hdlr(signal_t sig);


/* static variables */
static int dev_fd;


/* local functions */
/**
 *	\brief	test to verify the interrupt feature of gpio devices
 */
TEST_LONG(gpioint, "test gpio device interrupts"){
	char c;
	f_mode_t f_mode;
	gpio_int_cfg_t cfg;


	/* prepare */
	// open device
	dev_fd = open(GPIO_DEV, O_RDWR);

	if(dev_fd < 0){
		printf(FG_RED "error " RESET_ATTR "opening device \"%s\" \"%s\"\n", GPIO_DEV, strerror(errno));
		return -1;
	}

	// register signal handler
	if(signal(SIGNAL, hdlr) != hdlr){
		printf(FG_RED "error " RESET_ATTR "registering signal handler \"%s\"\n", strerror(errno));
		return -1;
	}

	// register signal to device interrupt
	cfg.mask = 0xff;
	cfg.sig = SIGNAL;

	if(ioctl(dev_fd, IOCTL_CFGWR, &cfg) != 0){
		printf(FG_RED "error " RESET_ATTR "registering signal to device \"%s\"\n", strerror(errno));
		return -1;
	}

	// make stdin non-blocking
	fcntl(0, F_MODE_GET, &f_mode, sizeof(f_mode_t));
	f_mode |= O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	/* wait for user to end the test */
	printf("use 'q' to quit the test\n");

	while(1){
		fread(&c, 1, stdin);

		if(c == 'q')
			break;

		sleep(10, 0);
	}

	/* cleanup */
	// close device
	close(dev_fd);

	// restore stdin blocking mode
	f_mode &= ~O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	return 0;
}

static void hdlr(signal_t sig){
	char c;


	read(dev_fd, &c, 1);
	printf("caught signal %d, pin state %d\n", sig, (int)c);
}
