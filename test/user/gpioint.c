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
#define GPIO_DEV	"/dev/gpio2int0"
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
	gpio_sig_cfg_t cfg;


	/* prepare */
	// open device
	dev_fd = open(GPIO_DEV, O_RDWR);

	if(dev_fd < 0){
		printf(FG("error ", RED) "opening device \"%s\" \"%s\"\n", GPIO_DEV, strerror(errno));
		return -1;
	}

	// register signal handler
	if(signal(SIGNAL, hdlr) != 0){
		printf(FG("error ", RED) "registering signal handler \"%s\"\n", strerror(errno));
		return -1;
	}

	// register signal to device interrupt
	cfg.mask = (intgpio_t)-1;
	cfg.signum = SIGNAL;

	if(ioctl(dev_fd, IOCTL_CFGWR, &cfg) != 0){
		printf(FG("error ", RED) "registering signal to device \"%s\"\n", strerror(errno));
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
	cfg.mask = 0x0;

	if(ioctl(dev_fd, IOCTL_CFGWR, &cfg) != 0)
		printf(FG("error ", RED) "releasing signal \"%s\"\n", strerror(errno));

	signal(SIGNAL, 0x0);

	// close device
	close(dev_fd);

	// restore stdin blocking mode
	f_mode &= ~O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	return 0;
}

static void hdlr(signal_t sig){
	intgpio_t v;


	read(dev_fd, &v, sizeof(intgpio_t));
	printf("caught signal %d, pin state %#x\n", sig, v);
}
