/**
 * Copyright (C) 2022 Jan Nowotsch
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
#define MASK_INVAL(dtd, mask_name)	\
	(((dtd)->cfg.mask_name & (dtd)->dti->cfg->mask_name) != (dtd)->cfg.mask_name)


/* types */
typedef struct{
	gpio_port_cfg_t cfg;
	gpio_t *dti; /**< set by the driver */
} dt_data_t;


/* local/static prototypes */
static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_t *dti = (gpio_t*)dt_itf;
	devfs_ops_t ops;


	dtd->dti = dti;

	if(MASK_INVAL(dtd, in_mask) || MASK_INVAL(dtd, out_mask) || MASK_INVAL(dtd, int_mask))
		goto_errno(end, E_INVAL);

	ops.open = 0x0;
	ops.close = close;
	ops.read = dtd->cfg.in_mask ? read : 0x0;
	ops.write = dtd->cfg.out_mask ? write : 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;
	ops.mmap = 0x0;

	(void)devfs_dev_register(name, &ops, dtd);

end:
	return 0x0;
}

driver_probe("gpio,port", probe);

static int close(devfs_dev_t *dev, fs_filed_t *fd){
	if(gpio_sig_release(((dt_data_t*)dev->payload)->dti, fd) != 0)
		reset_errno();

	return 0;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dt_data_t *dtd = (dt_data_t*)dev->payload;


	if(n != sizeof(intgpio_t))
		goto_errno(err, E_LIMIT);

	*((intgpio_t*)buf) = gpio_read(dtd->dti, dtd->cfg.in_mask);

	return n;


err:
	return 0;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dt_data_t *dtd = (dt_data_t*)dev->payload;


	if(n != sizeof(intgpio_t))
		goto_errno(err, E_LIMIT);

	return (gpio_write(dtd->dti, *((intgpio_t*)buf), dtd->cfg.out_mask) != 0) ? 0 : n;


err:
	return 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	dt_data_t *dtd = (dt_data_t*)dev->payload;
	gpio_sig_cfg_t *scfg = (gpio_sig_cfg_t*)arg;
	gpio_port_cfg_t *pcfg = (gpio_port_cfg_t*)arg;


	switch(request){
	case IOCTL_CFGRD:
		switch(n){
		case sizeof(gpio_port_cfg_t):
			pcfg->in_mask = dtd->cfg.in_mask;
			pcfg->out_mask = dtd->cfg.out_mask;
			pcfg->int_mask = dtd->cfg.int_mask;
			break;

		case sizeof(gpio_sig_cfg_t):
			gpio_sig_probe(dtd->dti, fd, scfg);
			scfg->mask &= dtd->cfg.int_mask;
			break;

		default:
			return_errno(E_INVAL);
		}

		return 0;

	case IOCTL_CFGWR:
		if(n != sizeof(gpio_sig_cfg_t))
			return_errno(E_INVAL);

		if(scfg->mask == 0x0)
			return gpio_sig_release(dtd->dti, fd);

		return gpio_sig_register(dtd->dti, fd, scfg);

	default:
		return_errno(E_NOSUP);
	}
}
