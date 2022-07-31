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
	i2c_t *i2c;
	dt_data_t *cfg;
} dev_data_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *buf, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *data;
	devfs_ops_t ops;


	data = kmalloc(sizeof(dev_data_t));

	if(data == 0x0)
		goto err;

	data->i2c = dt_itf;
	data->cfg = dt_data;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;

	(void)devfs_dev_register(name, &ops, data);

	return 0x0;


err:
	return 0x0;
}

driver_probe("i2c", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	size_t r;
	dev_data_t *data;


	data = (dev_data_t*)dev->data;

	mutex_unlock(&dev->node->mtx);
	r = i2c_read(data->i2c, data->cfg->slave, buf, n);
	mutex_lock(&dev->node->mtx);

	return r;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	size_t r;
	dev_data_t *data;


	data = (dev_data_t*)dev->data;

	mutex_unlock(&dev->node->mtx);
	r = i2c_write(data->i2c, data->cfg->slave, buf, n);
	mutex_lock(&dev->node->mtx);

	return r;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *buf, size_t n){
	dev_data_t *data;


	data = (dev_data_t*)dev->data;

	if(n != sizeof(dt_data_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(buf, data->cfg, n);
		break;

	case IOCTL_CFGWR:
		memcpy(data->cfg, buf, n);
		break;

	default:
		goto_errno(err, E_NOSUP);
	}

	return E_OK;


err:
	return -errno;
}
