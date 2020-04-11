/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <kernel/memory.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/ringbuf.h>


/* local/static prototypes */
static int open(devfs_dev_t *dev, fs_filed_t *fd, f_mode_t mode);
static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);
static int fcntl(devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data);


/* local functions */
static int probe(char const *name, void *data, void *hw_itf){
	devfs_ops_t ops;
	ringbuf_t *b;


	/* init device buffer */
	b = kmalloc(sizeof(ringbuf_t) + CONFIG_LOOP_BUF_SIZE);

	if(b == 0x0)
		return -errno;

	ringbuf_init(b, (char*)b + sizeof(ringbuf_t), CONFIG_LOOP_BUF_SIZE);

	/* register device */
	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = fcntl;

	if(devfs_dev_register(name, &ops, 0, b) == 0x0)
		goto err;

	return E_OK;


err:
	kfree(b);
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
	n = ringbuf_read(dev->data, buf, n);
	DEBUG("copy %u from loop buffer \"%*.*s\"\n", n, n, n, buf);

	if(n == 0)
		goto_errno(err, E_END);

	return n;

err:
	return 0;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	n = ringbuf_write(dev->data, buf, n);

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
