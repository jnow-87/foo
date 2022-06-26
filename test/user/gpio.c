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
#define DEV_NORMAL	"/dev/gpio-normal"
#define DEV_STRICT	"/dev/gpio-strict"

#define IN_MASK		0x3f		// has to match the device tree configuration
#define OUT_MASK	0xfc		// has to match the device tree configuration
#define INVERT_MASK	0xff		// has to match the device tree configuration

#define EXPECT(v)	(((((v) ^ INVERT_MASK) & OUT_MASK) ^ INVERT_MASK) & IN_MASK)


/* local/static prototypes */
static int test_read(int fd, gpio_int_t expect);
static int test_write(int fd, gpio_int_t v, gpio_int_t expect);
static int test_ioctl(int fd, gpio_int_t mask, signal_t sig, errno_t expect);


/* local functions */
/**
 *	\brief	test to verify the gpio functions
 */
TEST_LONG(gpio, "test gpio interface"){
	int n;
	int fd_normal,
		fd_strict;
	gpio_int_t v;


	n = 0;
	set_errno(E_OK);

	/* prepare */
	ASSERT_INT_NEQ(fd_normal = open(DEV_NORMAL, O_RDWR), -1);
	ASSERT_INT_NEQ(fd_strict = open(DEV_STRICT, O_RDWR), -1);

	/* normal cases */
	n += test_write(fd_normal, 0xae, EXPECT(0xae));
	n += test_write(fd_normal, 0x0, EXPECT(0x0));
	n += test_ioctl(fd_normal, 0xff, SIG_USR0, E_OK);

	/* error cases */
	// read more than sizeof(gpio_int_t)
	n += TEST_INT_EQ(read(fd_normal, &v, sizeof(v) * 2), -1);
	n += TEST_INT_EQ(errno, E_LIMIT);
	set_errno(E_OK);

	// write more than sizeof(gpio_int_t)
	n += TEST_INT_EQ(write(fd_normal, &v, sizeof(v) * 2), -1);
	n += TEST_INT_EQ(errno, E_LIMIT);
	set_errno(E_OK);

	// write bits other than in the devices out_mask
	// shall be ok in normal mode but give an error in strict mode
	n += test_write(fd_normal, 0xff, EXPECT(0xff));

	v = 0xff;
	n += TEST_INT_EQ(write(fd_strict, &v, sizeof(v)), -1);
	n += TEST_INT_EQ(errno, E_INVAL);
	set_errno(E_OK);

	// ioctl invalid signal
	n += test_ioctl(fd_normal, 0xff, SIG_USR0 - 1, E_INVAL);
	n += test_ioctl(fd_normal, 0xff, SIG_MAX, E_INVAL);
	n += test_ioctl(fd_strict, 0xff, SIG_USR0, E_INVAL);

	/* cleanup */
	// close device
	close(fd_normal);
	close(fd_strict);

	return -n;
}

static int test_read(int fd, gpio_int_t expect){
	int n;
	gpio_int_t v;


	v = ~expect;
	n = 0;

	n += TEST_INT_EQ(read(fd, &v, sizeof(v)), sizeof(v));
	n += TEST_INT_EQ(v, expect);

	return -n;
}

static int test_write(int fd, gpio_int_t v, gpio_int_t expect){
	int n;


	n = 0;
	n += TEST_INT_EQ(write(fd, &v, sizeof(v)), sizeof(v));
	n += test_read(fd, expect);

	return n;
}

static int test_ioctl(int fd, gpio_int_t mask, signal_t sig, errno_t expect){
	int n;
	gpio_int_cfg_t cfg;


	n = 0;
	cfg.mask = mask;
	cfg.sig = sig;

	n += TEST_INT_EQ(ioctl(fd, IOCTL_CFGWR, &cfg, sizeof(cfg)), expect ? -1 : 0);
	n += TEST_INT_EQ(errno, expect);
	set_errno(E_OK);

	return -n;
}
