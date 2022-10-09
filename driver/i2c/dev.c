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
#include <sys/mutex.h>
#include <sys/ioctl.h>
#include <sys/i2c.h>
#include <sys/string.h>


/* types */
typedef struct{
	uint8_t slave;
} dt_data_t;

typedef struct{
	i2c_t *itf;
	dt_data_t *cfg;
} dev_data_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *i2c;
	devfs_ops_t ops;


	i2c = kmalloc(sizeof(dev_data_t));

	if(i2c == 0x0)
		goto err;

	i2c->itf = dt_itf;
	i2c->cfg = dt_data;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.mmap = 0x0;

	(void)devfs_dev_register(name, &ops, i2c);

	return 0x0;


err:
	return 0x0;
}

driver_probe("i2c", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dev_data_t *i2c;


	i2c = (dev_data_t*)dev->payload;

	mutex_unlock(&dev->node->mtx);

	if(i2c_read(i2c->itf, i2c->cfg->slave, buf, n) != 0)
		n = 0;

	mutex_lock(&dev->node->mtx);

	return n;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dev_data_t *i2c;


	i2c = (dev_data_t*)dev->payload;

	mutex_unlock(&dev->node->mtx);

	if(i2c_write(i2c->itf, i2c->cfg->slave, buf, n) != 0)
		n = 0;

	mutex_lock(&dev->node->mtx);

	return n;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	dev_data_t *i2c;


	i2c = (dev_data_t*)dev->payload;

	if(n != sizeof(dt_data_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(arg, i2c->cfg, n);
		break;

	case IOCTL_CFGWR:
		memcpy(i2c->cfg, arg, n);
		break;

	default:
		goto_errno(err, E_NOSUP);
	}

	return 0;


err:
	return -errno;
}
