/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/fs.h>
#include <kernel/devfs.h>
#include <kernel/memory.h>
#include <driver/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/i2c.h>
#include <sys/string.h>


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	devfs_ops_t ops;
	i2c_t *i2c;
	i2c_ops_t *prim;


	prim = (i2c_ops_t*)dt_itf;

	/* configure hardware */
	if(prim->configure(dt_data, false, prim->hw) != E_OK)
		goto err_0;

	/* allocate eeprom */
	i2c = i2c_create(prim, dt_data, prim->hw);

	if(i2c == 0x0)
		goto err_0;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;

	if(devfs_dev_register(name, &ops, i2c) == 0x0)
		goto err_1;

	return 0x0;


err_1:
	i2c_destroy(i2c);

err_0:
	return 0x0;
}

driver_probe("i2c", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	size_t r;
	i2c_t *i2c;


	i2c = (i2c_t*)dev->data;

	mutex_unlock(&dev->node->mtx);
	r = i2c_read(i2c, i2c->cfg->target_addr, buf, n);
	mutex_lock(&dev->node->mtx);

	return r;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	size_t r;
	i2c_t *i2c;


	i2c = (i2c_t*)dev->data;

	mutex_unlock(&dev->node->mtx);
	r = i2c_write(i2c, i2c->cfg->target_addr, buf, n);
	mutex_lock(&dev->node->mtx);

	return r;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	i2c_t *i2c;


	i2c = (i2c_t*)dev->data;

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, i2c->cfg, sizeof(i2c_cfg_t));
		break;

	case IOCTL_CFGWR:
		if(i2c->ops->configure(data, false, i2c->ops->hw) != E_OK)
			goto err;

		memcpy(i2c->cfg, data, sizeof(i2c_cfg_t));
		break;

	default:
		goto_errno(err, E_NOSUP);
	}

	return E_OK;


err:
	return -errno;
}
