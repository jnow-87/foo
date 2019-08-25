/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <driver/i2c.h>
#include <sys/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>


/* types */
typedef struct{
	i2c_itf_t *hw;
	i2c_cfg_t *cfg;
} i2c_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	devfs_ops_t ops;
	i2c_t *i2c;


	/* allocate eeprom */
	i2c = kmalloc(sizeof(i2c_t));

	if(i2c == 0x0)
		goto err_0;

	i2c->hw = dt_itf;
	i2c->cfg = dt_data;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;

	if(devfs_dev_register(name, &ops, O_NONBLOCK, i2c) == 0x0)
		goto err_1;

	return E_OK;


err_1:
	kfree(i2c);

err_0:
	return -errno;
}

device_probe("i2c", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	size_t r;
	i2c_t *i2c;
	i2c_itf_t *hw;


	i2c = (i2c_t*)dev->data;
	hw = i2c->hw;

	if(i2c->cfg->mode == I2C_MODE_MASTER)	r = hw->master_read(i2c->cfg->target_addr, buf, n, hw->data);
	else									r = hw->slave_read(buf, n, hw->data);

	return r;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	size_t r;
	i2c_t *i2c;
	i2c_itf_t *hw;


	i2c = (i2c_t*)dev->data;
	hw = i2c->hw;

	if(i2c->cfg->mode == I2C_MODE_MASTER)	r = hw->master_write(i2c->cfg->target_addr, buf, n, hw->data);
	else									r = hw->slave_write(buf, n, hw->data);

	return r;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	i2c_t *i2c;
	i2c_itf_t *hw;


	i2c = (i2c_t*)dev->data;
	hw = i2c->hw;

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, i2c->cfg, sizeof(i2c_cfg_t));
		break;

	case IOCTL_CFGWR:
		if(hw->configure(data, hw->data) != E_OK)
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
