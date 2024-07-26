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
#include <sys/blob.h>
#include <sys/errno.h>
#include <sys/i2c.h>
#include <sys/ioctl.h>
#include <sys/mutex.h>
#include <sys/string.h>
#include <sys/types.h>


/* types */
typedef struct{
	i2c_t *itf;
	i2c_dev_cfg_t cfg;
} dev_data_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t xfer(devfs_dev_t *dev, fs_filed_t *fd, i2c_cmd_t cmd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *i2c;
	devfs_ops_t ops;


	i2c = kmalloc(sizeof(dev_data_t));

	if(i2c == 0x0)
		goto err;

	i2c->itf = dt_itf;
	i2c->cfg.mode = I2C_MASTER;
	i2c->cfg.slave = 0;

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
	return xfer(dev, fd, I2C_READ, buf, n);
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return xfer(dev, fd, I2C_WRITE, buf, n);
}

static size_t xfer(devfs_dev_t *dev, fs_filed_t *fd, i2c_cmd_t cmd, void *buf, size_t n){
	dev_data_t *i2c = (dev_data_t*)dev->payload;


	mutex_unlock(&dev->node->mtx);
	n = i2c_xfer(i2c->itf, i2c->cfg.mode, cmd, i2c->cfg.slave, BLOBS(BLOB(buf, n)), 1);
	mutex_lock(&dev->node->mtx);

	return n;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	dev_data_t *i2c = (dev_data_t*)dev->payload;


	if(n != sizeof(i2c_dev_cfg_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(arg, &i2c->cfg, n);
		break;

	case IOCTL_CFGWR:
		memcpy(&i2c->cfg, arg, n);
		break;

	default:
		goto_errno(err, E_NOSUP);
	}

	return 0;


err:
	return -errno;
}
