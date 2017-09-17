#ifndef KERNEL_ROOTFS_H
#define KERNEL_ROOTFS_H


#include <kernel/fs.h>
#include <sys/types.h>


/* types */
typedef struct{
	char *data;

	size_t data_max,
		   data_used;
} rootfs_file_t;


/* external variables */
extern fs_node_t fs_root;


/* prototypes */
fs_node_t *rootfs_mkdir(char const *path, fs_ops_t *ops);
int rootfs_rmdir(fs_node_t *node);


#endif // KERNEL_ROOTFS_H
