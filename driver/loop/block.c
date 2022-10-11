/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/memory.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/math.h>
#include <sys/string.h>
#include <sys/loop.h>


/* types */
typedef struct{
	void *buf;
	loop_cfg_t *cfg;
} loop_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);
static void *mmap(devfs_dev_t *dev, fs_filed_t *fd, size_t n);

static size_t copy(loop_t *loop, fs_filed_t *fd, void *to, void *from, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	loop_cfg_t *dtd = (loop_cfg_t*)dt_data;
	loop_t *loop;
	devfs_ops_t ops;


	/* init device buffer */
	loop = kmalloc(sizeof(loop_t) + dtd->size);

	if(loop == 0x0)
		goto err;

	loop->buf = ((void*)loop) + sizeof(loop_t);
	loop->cfg = dtd;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;
	ops.mmap = mmap;

	if(devfs_dev_register(name, &ops, loop) == 0x0)
		goto err;

	return 0x0;


err:
	kfree(loop);

	return 0x0;
}

driver_probe("loop,block", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	loop_t *loop = (loop_t*)dev->payload;


	return copy(loop, fd, buf, loop->buf + fd->fp, n);
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	loop_t *loop = (loop_t*)dev->payload;


	return copy(loop, fd, loop->buf + fd->fp, buf, n);
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	loop_t *loop = (loop_t*)dev->payload;


	if(n != sizeof(loop_cfg_t))
		return_errno(E_INVAL);

	if(request != IOCTL_CFGRD)
		return_errno(E_NOSUP);

	memcpy(arg, loop->cfg, n);

	return 0;
}

static void *mmap(devfs_dev_t *dev, fs_filed_t *fd, size_t n){
	loop_t *loop = (loop_t*)dev->payload;


	if(n > loop->cfg->size)
		goto_errno(err, E_LIMIT);

	return ummap(loop->buf);


err:
	return 0x0;
}

static size_t copy(loop_t *loop, fs_filed_t *fd, void *to, void *from, size_t n){
	n = MIN(n, loop->cfg->size - fd->fp);

	memcpy(to, from, n);
	fd->fp += n;

	return n;
}
