/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <sys/errno.h>
#include "loop.h"


/* local/static prototypes */
static int open(devfs_dev_t *dev, fs_filed_t *fd, f_mode_t mode);
static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);
static int fcntl(devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data);


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	devfs_ops_t ops;
	loop_t *loop;


	/* init device buffer */
	loop = loop_create(dt_data);

	if(loop == 0x0)
		goto err;

	/* register device */
	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = fcntl;

	if(devfs_dev_register(name, &ops, 0, loop) == 0x0)
		goto err;

	return E_OK;


err:
	loop_destroy(loop);

	return -errno;
}

device_probe("loop", probe);

static int open(devfs_dev_t *dev, fs_filed_t *fd, f_mode_t mode){
	DEBUG("dummy callback for loop device\n");

	return E_OK;
}

static int close(devfs_dev_t *dev, fs_filed_t *fd){
	DEBUG("dummy callback for loop device\n");

	return E_OK;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	n = loop_read(dev->data, buf, n);

	DEBUG("copy %u from loop buffer \"%*.*s\"\n", n, n, n, buf);

	return n;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	n = loop_write(dev->data, buf, n);

	DEBUG("copy %u to buffer \"%*.*s\"\n", n, n, n, buf);

	return n;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	DEBUG("dummy callback for loop device\n");

	return E_OK;
}

static int fcntl(devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data){
	DEBUG("dummy callback for loop device\n");

	return E_OK;
}
