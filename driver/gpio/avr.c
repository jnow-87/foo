/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/avr/atmega.h>
#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <sys/types.h>
#include <sys/register.h>


/* types */
typedef enum{
	PORT_IN = 0x1,
	PORT_OUT = 0x2,
	PORT_INOUT = 0x3,
} port_dir_t;

typedef struct{
	uint8_t volatile pin,
					 ddr,
					 port;
} uart_reg_t;

typedef struct{
	uart_reg_t *port;

	uint8_t dir,
			offset,
			mask;
} gpio_devtree_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	gpio_devtree_t *regs;
	devfs_ops_t ops;


	regs = (gpio_devtree_t*)dt_data;

	/* configure port */
	regs->port->port |= regs->mask;

	if(regs->dir & PORT_OUT)
		regs->port->ddr |= regs->mask;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = regs->dir & PORT_IN ? read : 0x0;
	ops.write = regs->dir & PORT_OUT ? write : 0x0;
	ops.ioctl = 0x0;
	ops.fcntl = 0x0;

	if(devfs_dev_register(name, &ops, 0x0, dt_data) == 0x0)
		return -errno;

	return E_OK;
}

driver_device("avr,gpio", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	gpio_devtree_t *regs;
	uint8_t v;


	regs = (gpio_devtree_t*)dev->data;

	/* read port */
	v = regs->port->pin;
	*((char*)buf) = (v & regs->mask) >> regs->offset;

	DEBUG("port %s, mask %#hhx, val %#hhx %#hhx\n", dev->node->name, regs->mask, *((char*)buf), v);

	return 1;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	uint8_t v;
	gpio_devtree_t *regs;


	regs = (gpio_devtree_t*)dev->data;

	/* write port */
	v = regs->port->pin & ~regs->mask;
	v |= (*((char*)buf) << regs->offset) & regs->mask;

	regs->port->port = v;

	DEBUG("port %s, mask %#hhx, val %#hhx\n", dev->node->name, regs->mask, v);

	return 1;
}
