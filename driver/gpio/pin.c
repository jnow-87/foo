/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/devfs.h>
#include <kernel/driver.h>
#include <kernel/fs.h>
#include <driver/gpio.h>
#include <sys/errno.h>
#include <sys/gpio.h>
#include <sys/ioctl.h>
#include <sys/types.h>


/* local/static prototypes */
static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	gpio_cfg_t *dtd = (gpio_cfg_t*)dt_data;
	gpio_itf_t *dti = (gpio_itf_t*)dt_itf;
	gpio_t *gpio;
	devfs_dev_t *dev;
	devfs_ops_t ops;


	/* register devfs device */
	gpio = gpio_create(dti, dtd);

	if(gpio == 0x0)
		goto err_0;

	ops.open = 0x0;
	ops.close = close;
	ops.read = dtd->in_mask ? read : 0x0;
	ops.write = dtd->out_mask ? write : 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;
	ops.mmap = 0x0;

	dev = devfs_dev_register(name, &ops, gpio);

	if(dev == 0x0)
		goto err_1;

	/* configure device */
	if(gpio_configure(gpio) != 0)
		goto err_2;

	return 0x0;


err_2:
	devfs_dev_release(dev);

err_1:
	gpio_destroy(gpio);

err_0:
	return 0x0;
}

driver_probe("gpio,pin", probe);

static int close(devfs_dev_t *dev, fs_filed_t *fd){
	if(gpio_int_release(dev->payload, fd) != 0)
		reset_errno();

	return 0;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	*((bool*)buf) = (bool)gpio_read(dev->payload);

	return errno ? 0 : 1;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	gpio_t *gpio = (gpio_t*)dev->payload;


	return (gpio_write(gpio, *((bool*)buf) ? gpio->cfg->out_mask : 0x0) == 0) ? 1 : 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	gpio_t *gpio = (gpio_t*)dev->payload;
	gpio_int_cfg_t *icfg = (gpio_int_cfg_t*)arg;
	gpio_port_cfg_t *pcfg = (gpio_port_cfg_t*)arg;


	switch(request){
	case IOCTL_CFGRD:
		if(n != sizeof(gpio_port_cfg_t))
			return_errno(E_INVAL);

		pcfg->in_mask = (bool)gpio->cfg->in_mask;
		pcfg->out_mask = (bool)gpio->cfg->out_mask;
		pcfg->int_mask = (bool)gpio->cfg->int_mask;

		gpio_int_probe(gpio, fd, &pcfg->interrupt);
		pcfg->interrupt.mask = (bool)pcfg->interrupt.mask;

		return 0;

	case IOCTL_CFGWR:
		if(n != sizeof(gpio_int_cfg_t))
			return_errno(E_INVAL);

		switch(icfg->op){
		case GPIO_INT_REGISTER:
			icfg->mask = gpio->cfg->int_mask;

			return gpio_int_register(gpio, fd, icfg);

		case GPIO_INT_RELEASE:	return gpio_int_release(gpio, fd);
		default:				return_errno(E_INVAL);
		}
		break;

	default:
		return_errno(E_NOSUP);
	}
}
