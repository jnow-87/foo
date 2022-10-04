/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/init.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/devfs.h>
#include <kernel/process.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <sys/string.h>
#include <sys/stat.h>
#include <sys/list.h>


/* static variables */
static int devfs_id = 0;
static fs_node_t *devfs_root = 0x0;


/* local/static prototypes */
static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p);
static int close(fs_filed_t *fd, process_t *this_p);
static size_t read(fs_filed_t *fd, void *buf, size_t n);
static size_t write(fs_filed_t *fd, void *buf, size_t n);
static int ioctl(fs_filed_t *fd, int request, void *data, size_t n);
static int fcntl(fs_filed_t *fd, int cmd, void *data);
static void *mmap(fs_filed_t *fd, size_t n);


/* global functions */
devfs_dev_t *devfs_dev_register(char const *name, devfs_ops_t *ops, void *data){
	devfs_dev_t *dev;
	fs_node_t *node;


	INFO("register device \"%s\"\n", name);

	dev = kmalloc(sizeof(devfs_dev_t));

	if(dev == 0x0)
		goto err_0;

	node = fs_node_create(devfs_root, name, strlen(name), FT_CHR, dev, devfs_id);

	if(node == 0x0)
		goto err_1;

	dev->ops = *ops;
	dev->data = data;
	dev->node = node;

	return dev;


err_1:
	kfree(dev);

err_0:
	return 0x0;
}

int devfs_dev_release(devfs_dev_t *dev){
	fs_node_t *node;


	fs_lock();

	node = list_find(devfs_root->childs, data, dev);

	if(node == 0x0)
		goto_errno(err, E_INVAL);

	if(fs_node_destroy(node) != 0)
		goto err;

	fs_unlock();

	kfree(dev);

	return 0;


err:
	fs_unlock();

	return -errno;
}


/* local functions */
static int init(void){
	fs_ops_t ops;


	/* register file system */
	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = fcntl;
	ops.mmap = mmap;
	ops.node_find = 0x0;
	ops.node_rm = 0x0;

	devfs_id = fs_register(&ops);

	if(devfs_id < 0)
		goto err_0;

	/* allocate root node */
	devfs_root = rootfs_mkdir("/dev", fs_root->fs_id);

	if(devfs_root == 0x0)
		goto err_1;

	return 0;


err_1:
	fs_release(devfs_id);

err_0:
	return -errno;
}

kernel_init(2, init);

static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p){
	fs_filed_t *fd;
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)start->data;
	fd = fs_fd_alloc(start, this_p, mode);

	if(fd == 0x0)
		return -errno;

	if(dev->ops.open == 0x0)
		return fd->id;

	if(dev->ops.open(dev, fd, mode) != 0)
		goto err;

	return fd->id;


err:
	fs_fd_free(fd, this_p);

	return -errno;
}

static int close(fs_filed_t *fd, process_t *this_p){
	int r;
	devfs_dev_t *dev;


	fs_lock();

	r = 0;
	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.close != 0x0)
		r = dev->ops.close(dev, fd);

	if(r == 0)
		fs_fd_free(fd, this_p);

	fs_unlock();

	return r;
}

static size_t read(fs_filed_t *fd, void *buf, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.read != 0x0)
		return dev->ops.read(dev, fd, buf, n);

	set_errno(E_NOIMP);

	return 0;
}

static size_t write(fs_filed_t *fd, void *buf, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.write != 0x0)
		return dev->ops.write(dev, fd, buf, n);

	set_errno(E_NOIMP);

	return 0;
}

static int ioctl(fs_filed_t *fd, int request, void *data, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.ioctl == 0x0)
		return_errno(E_NOIMP);

	return dev->ops.ioctl(dev, fd, request, data, n);
}

static int fcntl(fs_filed_t *fd, int cmd, void *data){
	devfs_dev_t *dev;
	stat_t *stat;


	dev = (devfs_dev_t*)fd->node->data;

	switch(cmd){
	case F_STAT:
		stat = (stat_t*)data;

		stat->type = fd->node->type;
		stat->size = 0;

		return 0;

	default:
		if(dev->ops.fcntl == 0x0)
			return_errno(E_NOIMP);

		return dev->ops.fcntl(dev, fd, cmd, data);
	};
}

static void *mmap(fs_filed_t *fd, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t *)fd->node->data;

	if(dev->ops.mmap != 0x0)
		return dev->ops.mmap(dev, fd, n);

	set_errno(E_NOIMP);

	return 0x0;
}
