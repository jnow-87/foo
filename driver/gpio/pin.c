/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/devfs.h>
#include <kernel/driver.h>
#include <kernel/fs.h>
#include <kernel/memory.h>
#include <driver/gpio.h>
#include <sys/errno.h>
#include <sys/gpio.h>
#include <sys/ioctl.h>
#include <sys/types.h>


/* macros */
#define MASK_INVAL(pin, md, mask_name)	\
	(((pin)->dtd->mode & md) && (((0x1 << (pin)->dtd->pin) & (pin)->dti->cfg->mask_name) != (intgpio_t)(0x1 << (pin)->dtd->pin)))


/* types */
typedef enum{
	MODE_IN = 0x1,
	MODE_OUT = 0x2,
} mode_t;

typedef struct{
	uint8_t pin;
	uint8_t mode;	/**< cf. mode_t */
} dt_data_t;

typedef struct{
	dt_data_t *dtd;
	gpio_t *dti;
} dev_data_t;


/* local/static prototypes */
static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_t *dti = (gpio_t*)dt_itf;
	dev_data_t *pin;
	devfs_dev_t *dev;
	devfs_ops_t ops;


	pin = kmalloc(sizeof(dev_data_t));

	if(pin == 0x0)
		goto err_0;

	pin->dtd = dtd;
	pin->dti = dti;

	if(MASK_INVAL(pin, MODE_IN, in_mask) || MASK_INVAL(pin, MODE_OUT, out_mask))
		goto_errno(err_1, E_INVAL)

	ops.open = 0x0;
	ops.close = close;
	ops.read = (dtd->mode & MODE_IN) ? read : 0x0;
	ops.write = (dtd->mode & MODE_OUT) ? write : 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;
	ops.mmap = 0x0;

	dev = devfs_dev_register(name, &ops, pin);

	if(dev == 0x0)
		goto err_1;

	return 0x0;


err_1:
	kfree(pin);

err_0:
	return 0x0;
}

driver_probe("gpio,pin", probe);

static int close(devfs_dev_t *dev, fs_filed_t *fd){
	if(gpio_sig_release(((dev_data_t*)dev->payload)->dti, fd) != 0)
		reset_errno();

	return 0;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dev_data_t *pin = (dev_data_t*)dev->payload;


	*((bool*)buf) = (bool)gpio_read(pin->dti, (0x1 << pin->dtd->pin));

	return errno ? 0 : 1;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dev_data_t *pin = (dev_data_t*)dev->payload;
	intgpio_t v = ((bool)(*((uint8_t*)buf)) << pin->dtd->pin);


	return (gpio_write(pin->dti, v, (0x1 << pin->dtd->pin)) == 0) ? 1 : 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	dev_data_t *pin = (dev_data_t*)dev->payload;
	gpio_sig_cfg_t *scfg = (gpio_sig_cfg_t*)arg;


	switch(request){
	case IOCTL_CFGRD:
		if(n != sizeof(gpio_sig_cfg_t))
			return_errno(E_INVAL);

		gpio_sig_probe(pin->dti, fd, scfg);
		scfg->mask = (bool)scfg->mask;

		return 0;

	case IOCTL_CFGWR:
		if(n != sizeof(gpio_sig_cfg_t))
			return_errno(E_INVAL);

		if(scfg->mask == 0x0)
			return gpio_sig_release(pin->dti, fd);

		scfg->mask = (0x1 << pin->dtd->pin);

		return gpio_sig_register(pin->dti, fd, scfg);


	default:
		return_errno(E_NOSUP);
	}
}
