#include <kernel/init.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/devfs.h>
#include <kernel/kmem.h>
#include <kernel/kprintf.h>
#include <sys/list.h>


/* static variables */
static fs_ops_t devfs_ops;
static fs_node_t *devfs_root = 0x0;


/* local/static prototypes */
static int open(fs_node_t *start, char const *path, f_mode_t mode);
static int close(fs_filed_t *fd);
static size_t read(fs_filed_t *fd, void *buf, size_t n);
static size_t write(fs_filed_t *fd, void *buf, size_t n);
static int ioctl(fs_filed_t *fd, int request, void *data);
static int fcntl(fs_filed_t *fd, int cmd, void *data);



/* global functions */
int devfs_dev_register(char const *name, devfs_ops_t *ops){
	static int id = 0;
	devfs_dev_t *dev;
	fs_node_t *node;


	DEBUG("register device \"%s\"\n", name);

	if(id < 0)
		return_errno(E_LIMIT);

	dev = kmalloc(sizeof(devfs_dev_t));

	if(dev == 0x0)
		goto_errno(err_0, E_NOMEM);

	node = fs_node_alloc(devfs_root, name, strlen(name), false, &devfs_ops);

	if(node == 0x0)
		goto err_1;

	dev->id = id;
	dev->ops = *ops;
	node->data = dev;

	++id;

	return dev->id;


err_1:
	kfree(dev);

err_0:
	return errno;
}

int devfs_dev_release(int id){
	fs_node_t *node;
	devfs_dev_t *dev;


	list_for_each(devfs_root->childs, node){
		if(((devfs_dev_t*)(node->data))->id == id)
			break;
	}

	if(node == 0x0)
		return_errno(E_INVAL);

	dev = (devfs_dev_t*)node->data;

	if(fs_node_free(node) != E_OK)
		return errno;

	kfree(dev);

	return E_OK;
}


/* static functions */
static int init(void){
	devfs_ops.open = open;
	devfs_ops.close = close;
	devfs_ops.read = read;
	devfs_ops.write = write;
	devfs_ops.ioctl = ioctl;
	devfs_ops.fcntl = fcntl;

	devfs_root = rootfs_mkdir("/dev", fs_root.ops);

	if(devfs_root == 0x0)
		return errno;

	return E_OK;
}

kernel_init(2, init);

static int open(fs_node_t *start, char const *path, f_mode_t mode){
	fs_filed_t *fd;
	devfs_dev_t *dev;


	DEBUG("open device \"%s\"\n", start->name);

	fd = fs_fd_alloc(start);

	if(fd == 0x0)
		return errno;

	dev = (devfs_dev_t*)start->data;

	if(dev->ops.open == 0x0)
		return fd->id;

	if(dev->ops.open(dev->id, fd, mode) != E_OK)
		goto err;

	return fd->id;


err:
	fs_fd_free(fd);
	return errno;
}

static int close(fs_filed_t *fd){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.close != 0x0){
		if(dev->ops.close(dev->id, fd) != E_OK)
			return errno;
	}

	fs_fd_free(fd);
	return 0;
}

static size_t read(fs_filed_t *fd, void *buf, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.read == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.read(dev->id, fd, buf, n);
}

static size_t write(fs_filed_t *fd, void *buf, size_t n){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.write == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.write(dev->id, fd, buf, n);
}

static int ioctl(fs_filed_t *fd, int request, void *data){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.ioctl == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.ioctl(dev->id, fd, request, data);
}

static int fcntl(fs_filed_t *fd, int cmd, void *data){
	devfs_dev_t *dev;


	dev = (devfs_dev_t*)fd->node->data;

	if(dev->ops.fcntl == 0x0)
		return_errno(E_NOIMP);
	return dev->ops.fcntl(dev->id, fd, cmd, data);
}
