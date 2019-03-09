/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/init.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <kernel/memory.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/ringbuf.h>


/* local variables */
static ringbuf_t dev_buf;


/* local/static prototypes */
static int open(devfs_dev_t *dev, fs_filed_t *fd, f_mode_t mode);
static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);
static int fcntl(devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data);


/* local functions */
static int init(void){
	char *b;
	devfs_ops_t ops;


	/* init device buffer */
	b = kmalloc(CONFIG_LOOP_BUF_SIZE);

	if(b == 0x0)
		return_errno(E_NOMEM);

	ringbuf_init(&dev_buf, b, CONFIG_LOOP_BUF_SIZE);

	/* register device */
	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = fcntl;

	if(devfs_dev_register("loop", &ops, 0, 0x0) == 0x0)
		goto err;

	return E_OK;


err:
	kfree(b);
	return -errno;
}

driver_init(init);

static int open(devfs_dev_t *dev, fs_filed_t *fd, f_mode_t mode){
	DEBUG("dummy callback for loop device\n");
	return E_OK;
}

static int close(devfs_dev_t *dev, fs_filed_t *fd){
	DEBUG("dummy callback for loop device\n");
	return E_OK;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	n = ringbuf_read(&dev_buf, buf, n);
	DEBUG("copy from loop buffer \"%*.*s\"\n", n, n, buf);

	return n;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	n = ringbuf_write(&dev_buf, buf, n);

	DEBUG("copy to buffer \"%*.*s\"\n", n, n, buf);

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
