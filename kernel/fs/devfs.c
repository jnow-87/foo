#include <kernel/init.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/devfs.h>
#include <kernel/kmem.h>
#include <kernel/kprintf.h>
#include <kernel/lock.h>
#include <sys/list.h>


/* static variables */
static int devfs_id;
static fs_node_t *devfs_root = 0x0;


/* local/static prototypes */
static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p);
static int close(fs_filed_t *fd, process_t *this_p);
static size_t read(fs_filed_t *fd, void *buf, size_t n);
static size_t write(fs_filed_t *fd, void *buf, size_t n);
static int ioctl(fs_filed_t *fd, int request, void *data);
static int fcntl(fs_filed_t *fd, int cmd, void *data);



/* global functions */
devfs_dev_t *devfs_dev_register(char const *name, devfs_ops_t *ops, void *data){
	devfs_dev_t *dev;
	fs_node_t *node;


	DEBUG("register device \"%s\"\n", name);

	dev = kmalloc(sizeof(devfs_dev_t));

	if(dev == 0x0)
		goto_errno(err_0, E_NOMEM);

	node = fs_node_alloc(devfs_root, name, strlen(name), false, devfs_id);

	if(node == 0x0)
		goto err_1;

	dev->ops = *ops;
	dev->data = data;
	node->data = dev;

	return dev;


err_1:
	kfree(dev);

err_0:
	return 0x0;
}

int devfs_dev_release(devfs_dev_t *dev){
	fs_node_t *node;


	klock();

	list_for_each(devfs_root->childs, node){
		if(((devfs_dev_t*)(node->data)) == dev)
			break;
	}

	kunlock();

	if(node == 0x0)
		return_errno(E_INVAL);

	if(fs_node_free(node) != E_OK)
		return errno;

	kfree(dev);

	return E_OK;
}


/* static functions */
static int init(void){
	fs_ops_t ops;


	/* register file system */
	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = fcntl;

	devfs_id = fs_register(&ops);

	if(devfs_id < 0)
		return errno;

	/* allocate root node */
	devfs_root = rootfs_mkdir("/dev", devfs_id);

	if(devfs_root == 0x0)
		return errno;

	devfs_root->ops = fs_root->ops;	// use rootfs callbacks to handle devfs root

	return E_OK;
}

kernel_init(2, init);

static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p){
	fs_filed_t *fd;
	devfs_dev_t *dev;


	DEBUG("open device \"%s\"\n", start->name);

	fd = fs_fd_alloc(start, this_p);

	if(fd == 0x0)
		return errno;

	dev = (devfs_dev_t*)start->data;

	if(dev->ops.open == 0x0)
		return fd->id;

	if(dev->ops.open(dev, fd, mode) != E_OK)
		goto err;

	return fd->id;


err:
	fs_fd_free(fd, this_p);
	return errno;
}

static int close(fs_filed_t *fd, process_t *this_p){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.close != 0x0){
		if(dev->ops.close(dev, fd) != E_OK)
			return errno;
	}

	fs_fd_free(fd, this_p);
	return 0;
}

static size_t read(fs_filed_t *fd, void *buf, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.read == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.read(dev, fd, buf, n);
}

static size_t write(fs_filed_t *fd, void *buf, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.write == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.write(dev, fd, buf, n);
}

static int ioctl(fs_filed_t *fd, int request, void *data){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.ioctl == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.ioctl(dev, fd, request, data);
}

static int fcntl(fs_filed_t *fd, int cmd, void *data){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.fcntl == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.fcntl(dev, fd, cmd, data);
}
