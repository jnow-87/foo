#ifndef KERNEL_DEVFS_H
#define KERNEL_DEVFS_H


#include <kernel/fs.h>
#include <sys/file.h>


/* types */
typedef struct{
	int (*open)(int id, fs_filed_t *fd, f_mode_t mode);
	int (*close)(int id);
	int (*read)(int id, fs_filed_t *fd, void *buf, size_t n);
	int (*write)(int id, fs_filed_t *fd, void *buf, size_t n);
	int (*ioctl)(int id, fs_filed_t *fd, int request, void *data);
	int (*fcntl)(int id, fs_filed_t *fd, int cmd, void *data);
} devfs_ops_t;

typedef struct{
	int id;
	devfs_ops_t ops;
} devfs_dev_t;


/* prototypes */
int devfs_dev_register(char const *name, devfs_ops_t *ops);
int devfs_dev_release(int id);

#endif // KERNEL_DEVFS_H
