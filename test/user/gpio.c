/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <signal.h>
#include <unistd.h>
#include <sys/devtree.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/gpio.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/stream.h>
#include <sys/types.h>
#include <test/test.h>


/* macros */
#define INT_MAGIC	0xfade

#define DEV_PORT	"gpio-port"
#define DEV_PIN		"gpio-pin"
#define DEV_INT		"gpio-int"

#define SIGNAL		SIG_USR1
#define SIGNAL_MAX	3


/* types */
typedef struct{
	intgpio_t in_mask,
			  out_mask,
			  int_mask,
			  invert_mask;

	uint8_t int_num;
} gpio_cfg_t;


/* local/static prototypes */
static int setup(char const *dev_name, int *fd, gpio_cfg_t **cfg);

static int test_read(int fd, intgpio_t expect, size_t size, gpio_cfg_t *cfg);
static int test_write(int fd, intgpio_t v, intgpio_t expect, size_t size, gpio_cfg_t *cfg);
static int test_ioctl(int fd, ioctl_cmd_t cmd, void *cfg, size_t size, errno_t errnum);

static void sig_hdlr(signal_t sig);


/* static variables */
static int caught_sigs = 0;


/* local functions */
TEST(gpio_port_rw){
	int r = 0;
	int fd;
	intgpio_t v;
	gpio_cfg_t *dt_cfg;


	ASSERT_INT_NEQ(setup(DEV_PORT, &fd, &dt_cfg), -1);

	/* normal cases */
	// write according to out_mask
	r |= TEST_INT_EQ(0x0f00 & ~(dt_cfg->out_mask), 0x0);
	r |= test_write(fd, 0x0f00, 0x0f00, sizeof(intgpio_t), dt_cfg);

	// write outside of out_mask
	r |= TEST_INT_NEQ(0xff0f00 & ~(dt_cfg->out_mask), 0x0);
	r |= test_write(fd, 0xff0f00, 0x0f00, sizeof(intgpio_t), dt_cfg);

	/* error cases */
	// read more than sizeof(intgpio_t)
	r |= TEST_INT_EQ(read(fd, &v, sizeof(v) * 2), -1);
	r |= TEST_INT_EQ(errno, E_LIMIT);
	reset_errno();

	// write more than sizeof(intgpio_t)
	r |= TEST_INT_EQ(write(fd, &v, sizeof(v) * 2), -1);
	r |= TEST_INT_EQ(errno, E_LIMIT);
	reset_errno();

	/* cleanup */
	r |= TEST_INT_EQ(close(fd), 0);

	return -r;
}

TEST(gpio_port_ioctl){
	int r = 0;
	int fd;
	gpio_cfg_t *dt_cfg;
	gpio_port_cfg_t pcfg;


	ASSERT_INT_NEQ(setup(DEV_PORT, &fd, &dt_cfg), -1);

	/* normal cases */
	// read port config
	r |= test_ioctl(fd, IOCTL_CFGRD, &pcfg, sizeof(pcfg), 0);
	r |= TEST_INT_EQ(pcfg.in_mask, dt_cfg->in_mask);
	r |= TEST_INT_EQ(pcfg.out_mask, dt_cfg->out_mask);
	r |= TEST_INT_EQ(pcfg.int_mask, dt_cfg->int_mask);
	r |= TEST_INT_EQ(pcfg.interrupt.mask, 0);
	r |= TEST_INT_EQ(pcfg.interrupt.signum, 0);

	// write interrupt config
	r |= TEST_INT_NEQ(0xff0f & ~dt_cfg->int_mask, 0x0);
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = 0xff0f, .signum = SIG_USR0 }, sizeof(gpio_int_cfg_t), 0);

	// read port config again
	r |= test_ioctl(fd, IOCTL_CFGRD, &pcfg, sizeof(pcfg), 0);
	r |= TEST_INT_EQ(pcfg.in_mask, dt_cfg->in_mask);
	r |= TEST_INT_EQ(pcfg.out_mask, dt_cfg->out_mask);
	r |= TEST_INT_EQ(pcfg.int_mask, dt_cfg->int_mask);
	r |= TEST_INT_EQ(pcfg.interrupt.mask, dt_cfg->int_mask);
	r |= TEST_INT_EQ(pcfg.interrupt.signum, SIG_USR0);

	// overwrite interrupt config
	r |= TEST_INT_NEQ(0xff0e & ~dt_cfg->int_mask, 0x0);
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = 0xff0e, .signum = SIG_USR1 }, sizeof(gpio_int_cfg_t), 0);

	// read port config again
	r |= test_ioctl(fd, IOCTL_CFGRD, &pcfg, sizeof(pcfg), 0);
	r |= TEST_INT_EQ(pcfg.interrupt.mask, 0xff0e & dt_cfg->int_mask);
	r |= TEST_INT_EQ(pcfg.interrupt.signum, SIG_USR1);

	/* error cases */
	// invalid sizes
	r |= test_ioctl(fd, IOCTL_CFGRD, &pcfg, sizeof(pcfg) - 1, E_INVAL);
	r |= test_ioctl(fd, IOCTL_CFGWR, &pcfg, sizeof(pcfg) - 1, E_INVAL);

	// invalid signal
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = 0xff0f, .signum = SIG_USR0 - 1 }, sizeof(gpio_int_cfg_t), E_INVAL);
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = 0xff0f, .signum = SIG_MAX }, sizeof(gpio_int_cfg_t), E_INVAL);

	/* cleanup */
	r |= TEST_INT_EQ(close(fd), 0);

	return -r;
}

TEST(gpio_pin_rw){
	int r = 0;
	gpio_cfg_t cfg = {		// fake config to avoid test_read() masking the lowest bit
		.in_mask = 0xffff,	// which would make it impossible to compare the boolean
		.out_mask = 0xffff,	// values returned by pin devices
	};
	int fd;
	gpio_cfg_t *dt_cfg;


	ASSERT_INT_NEQ(setup(DEV_PIN, &fd, &dt_cfg), -1);

	/* normal cases */
	r |= test_write(fd, 0x00, 0x0, 1, &cfg);
	r |= test_write(fd, 0x01, 0x1, 1, &cfg);
	r |= test_write(fd, 0xf0, 0x1, 1, &cfg);

	/* cleanup */
	r |= TEST_INT_EQ(close(fd), 0);

	return -r;
}

TEST(gpio_pin_ioctl){
	int r = 0;
	int fd;
	gpio_cfg_t *dt_cfg;
	gpio_port_cfg_t pcfg;


	ASSERT_INT_NEQ(setup(DEV_PIN, &fd, &dt_cfg), -1);

	/* normal cases */
	// read port config
	r |= test_ioctl(fd, IOCTL_CFGRD, &pcfg, sizeof(pcfg), 0);
	r |= TEST_INT_EQ(pcfg.in_mask, 0x1);
	r |= TEST_INT_EQ(pcfg.out_mask, 0x1);
	r |= TEST_INT_EQ(pcfg.int_mask, 0x1);
	r |= TEST_INT_EQ(pcfg.interrupt.mask, 0);
	r |= TEST_INT_EQ(pcfg.interrupt.signum, 0);

	// write interrupt config
	r |= TEST_INT_NEQ(0xff0f & ~dt_cfg->int_mask, 0x0);
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = 0xff0f, .signum = SIG_USR0 }, sizeof(gpio_int_cfg_t), 0);

	// read port config again
	r |= test_ioctl(fd, IOCTL_CFGRD, &pcfg, sizeof(pcfg), 0);
	r |= TEST_INT_EQ(pcfg.in_mask, 0x1);
	r |= TEST_INT_EQ(pcfg.out_mask, 0x1);
	r |= TEST_INT_EQ(pcfg.int_mask, 0x1);
	r |= TEST_INT_EQ(pcfg.interrupt.mask, 0x1);
	r |= TEST_INT_EQ(pcfg.interrupt.signum, SIG_USR0);

	/* error cases */
	// invalid sizes
	r |= test_ioctl(fd, IOCTL_CFGRD, &pcfg, sizeof(pcfg) - 1, E_INVAL);
	r |= test_ioctl(fd, IOCTL_CFGWR, &pcfg, sizeof(pcfg) - 1, E_INVAL);

	// invalid signal
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = 0xff0f, .signum = SIG_USR0 - 1 }, sizeof(gpio_int_cfg_t), E_INVAL);
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = 0xff0f, .signum = SIG_MAX }, sizeof(gpio_int_cfg_t), E_INVAL);

	/* cleanup */
	r |= TEST_INT_EQ(close(fd), 0);

	return -r;
}

TEST(gpio_interrupt){
	int r = 0;
	int fd;
	gpio_cfg_t *dt_cfg;


	/* init */
	caught_sigs = 0;
	ASSERT_INT_NEQ(setup(DEV_INT, &fd, &dt_cfg), -1);

	// register signal
	ASSERT_INT_EQ(signal(SIGNAL, sig_hdlr), 0);
	r |= test_ioctl(fd, IOCTL_CFGWR, &(gpio_int_cfg_t){ .op = GPIO_INT_REGISTER, .mask = dt_cfg->int_mask, .signum = SIGNAL }, sizeof(gpio_int_cfg_t), 0);

	// enable test interrupt
	r |= TEST_INT_EQ(write(fd, &((intgpio_t){ INT_MAGIC }), sizeof(intgpio_t)), sizeof(intgpio_t));

	/* wait for signals */
	for(size_t i=0; i<100 && caught_sigs<SIGNAL_MAX; i++){
		r |= TEST_INT_EQ(sleep(10, 0), 0);
	}

	r |= TEST_INT_EQ(caught_sigs, SIGNAL_MAX);

	/* cleanup */
	// disable test interrupt
	r |= TEST_INT_EQ(write(fd, &((intgpio_t){ INT_MAGIC }), sizeof(intgpio_t)), sizeof(intgpio_t));

	r |= TEST_INT_EQ(signal(SIGNAL, 0x0), 0);
	r |= TEST_INT_EQ(close(fd), 0);


	return -r;
}

static int setup(char const *dev_name, int *fd, gpio_cfg_t **cfg){
	char name[strlen(dev_name) + 6];
	devtree_device_t const *dt_node;


	reset_errno();

	sprintf(name, "/dev/%s", dev_name);
	*fd = open(name, O_RDWR);

	if(*fd == -1)
		return -1;

	dt_node = devtree_find_device_by_name(&__dt_device_root, dev_name);

	if(dt_node == 0x0)
		return -1;

	*cfg = (gpio_cfg_t*)dt_node->payload;

	return 0;
}

static int test_read(int fd, intgpio_t expect, size_t size, gpio_cfg_t *cfg){
	int r = 0;
	intgpio_t v = 0;


	expect = ((expect & cfg->out_mask) ^ cfg->invert_mask) & cfg->in_mask;

	r |= TEST_INT_EQ(read(fd, &v, sizeof(v)), size);
	r |= TEST_INT_EQ(v, expect);

	return -r;
}

static int test_write(int fd, intgpio_t v, intgpio_t expect, size_t size, gpio_cfg_t *cfg){
	int r = 0;


	r |= TEST_INT_EQ(write(fd, &v, sizeof(v)), size);
	r |= test_read(fd, expect, size, cfg);

	return -r;
}

static int test_ioctl(int fd, ioctl_cmd_t cmd, void *cfg, size_t size, errno_t errnum){
	int r = 0;


	r |= TEST_INT_EQ(ionctl(fd, cmd, cfg, size), errnum ? -1 : 0);
	r |= TEST_INT_EQ(errno, errnum);
	reset_errno();

	return -r;
}

static void sig_hdlr(signal_t sig){
	if(caught_sigs < SIGNAL_MAX)
		caught_sigs++;
}
