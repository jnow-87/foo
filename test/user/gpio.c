/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/gpio.h>
#include <test/test.h>


/* macros */
#define DEV_NAME	"/dev/gpio"

#define IN_MASK		0x3f		// has to match the device tree configuration
#define OUT_MASK	0xfc		// has to match the device tree configuration
#define INVERT_MASK	0xff		// has to match the device tree configuration

#define EXPECT(v)	(((((v) ^ INVERT_MASK) & OUT_MASK) ^ INVERT_MASK) & IN_MASK)


/* local/static prototypes */
static int test_read(int fd, intgpio_t expect);
static int test_write(int fd, intgpio_t v, intgpio_t expect);
static int test_ioctl(int fd, intgpio_t mask, signal_t sig, errno_t expect);


/* local functions */
/**
 *	\brief	test to verify the gpio functions
 */
TEST_LONG(gpio, "test gpio interface"){
	int r = 0;
	int fd;
	intgpio_t v;


	reset_errno();

	/* prepare */
	ASSERT_INT_NEQ(fd = open(DEV_NAME, O_RDWR), -1);

	/* normal cases */
	r += test_write(fd, 0xae, EXPECT(0xae));
	r += test_write(fd, 0x0, EXPECT(0x0));
	r += test_ioctl(fd, 0xff, SIG_USR0, 0);

	/* error cases */
	// read more than sizeof(intgpio_t)
	r += TEST_INT_EQ(read(fd, &v, sizeof(v) * 2), -1);
	r += TEST_INT_EQ(errno, E_LIMIT);
	reset_errno();

	// write more than sizeof(intgpio_t)
	r += TEST_INT_EQ(write(fd, &v, sizeof(v) * 2), -1);
	r += TEST_INT_EQ(errno, E_LIMIT);
	reset_errno();

	// write bits other than in the devices out_mask shall still be ok
	r += test_write(fd, 0xff, EXPECT(0xff));

	// ioctl invalid signal
	r += test_ioctl(fd, 0xff, SIG_USR0 - 1, E_INVAL);
	r += test_ioctl(fd, 0xff, SIG_MAX, E_INVAL);

	/* cleanup */
	// close device
	r += TEST_INT_EQ(close(fd), 0);

	return -r;
}

static int test_read(int fd, intgpio_t expect){
	int r = 0;
	intgpio_t v;


	v = ~expect;

	r += TEST_INT_EQ(read(fd, &v, sizeof(v)), sizeof(v));
	r += TEST_INT_EQ(v, expect);

	return -r;
}

static int test_write(int fd, intgpio_t v, intgpio_t expect){
	int r = 0;


	r += TEST_INT_EQ(write(fd, &v, sizeof(v)), sizeof(v));
	r += test_read(fd, expect);

	return r;
}

static int test_ioctl(int fd, intgpio_t mask, signal_t sig, errno_t expect){
	int r = 0;
	gpio_int_cfg_t cfg;


	cfg.op = GPIO_INT_REGISTER;
	cfg.mask = mask;
	cfg.signum = sig;

	r += TEST_INT_EQ(ioctl(fd, IOCTL_CFGWR, &cfg), expect ? -1 : 0);
	r += TEST_INT_EQ(errno, expect);
	reset_errno();

	return -r;
}
