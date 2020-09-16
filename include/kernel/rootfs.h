/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



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


/* prototypes */
fs_node_t *rootfs_mkdir(char const *path, int fs_id);
int rootfs_rmdir(fs_node_t *node);


/* external variables */
extern fs_node_t *fs_root;


#endif // KERNEL_ROOTFS_H
