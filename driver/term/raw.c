/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <kernel/devfs.h>
#include <kernel/driver.h>
#include <driver/term.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>
#include <sys/types.h>


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_t *dti = (term_t*)dt_itf;
	devfs_ops_t ops = { 0x0 };
	devfs_dev_t *dev;


	/* register device */
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;

	dev = devfs_dev_register(name, &ops, dti);

	if(dev == 0x0)
		return 0x0;

	dti->node = dev->node;

	if(dti->itf->rx_int)
		dti->node->timeout_us = 0;

	return 0x0;
}

driver_probe("terminal,raw", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return term_gets(dev->payload, buf, n);
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return term_puts(dev->payload, buf, n);
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	term_t *term = (term_t*)dev->payload;


	if(n != sizeof(term_cfg_t) && n != (sizeof(term_cfg_t) + term->itf->cfg_size))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(arg, term->cfg, sizeof(term_cfg_t));

		if(n > sizeof(term_cfg_t))
			memcpy(arg + sizeof(term_cfg_t), term->itf->cfg, term->itf->cfg_size);

		return 0;

	case IOCTL_CFGWR:
		if(n > sizeof(term_cfg_t)){
			if(term->itf->configure == 0x0)
				return_errno(E_NOSUP);

			if(term->itf->configure(arg, arg + sizeof(term_cfg_t), term->itf->hw) != 0)
				return -errno;

			memcpy(term->itf->cfg, arg + sizeof(term_cfg_t), term->itf->cfg_size);
		}

		memcpy(term->cfg, arg, sizeof(term_cfg_t));

		return 0;

	default:
		return_errno(E_NOSUP);
	}
}
