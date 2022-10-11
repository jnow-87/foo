/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/process.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/stat.h>


/* local/static prototypes */
static int dump_node(fs_node_t *node);
static int dump_file(fs_node_t *node);


/* global functions */
int x86_rootfs_dump(void){
	INFO("export file system to %s\n", CONFIG_TEST_INT_FS_EXPORT_ROOT);

	return dump_node(fs_root);
}


/* local functions */
static int dump_node(fs_node_t *node){
	int r;
	char const *name;
	fs_node_t *child;


	switch(node->type){
	case FT_DIR:
		name = (node == fs_root) ? CONFIG_TEST_INT_FS_EXPORT_ROOT : node->name;

		lnx_mkdir(name, 511);
		lnx_chdir(name);

		r = 0;

		list_for_each(node->childs, child)
			r |= dump_node(child);

		lnx_chdir("..");

		return r;

	case FT_REG:
		return dump_file(node);

	default:
		return 0;
	}
}

static int dump_file(fs_node_t *node){
	int bos_id,
		lnx_id;
	size_t n;
	char buf[32];
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	bos_id = node->ops->open(node->parent, node->name, O_RDONLY, this_p);
	fd = fs_fd_acquire(bos_id, this_p);

	if(bos_id < 0 || fd == 0x0)
		return -1;

	lnx_id = lnx_open(node->name, LNX_O_RDWR | LNX_O_CREAT, 0666);

	if(lnx_id < 0)
		return -1;

	while(1){
		n = node->ops->read(fd, buf, sizeof(buf));

		if(n <= 0)
			break;

		lnx_write(lnx_id, buf, n);
	}

	fs_fd_release(fd);
	(void)node->ops->close(fd, this_p);
	lnx_close(lnx_id);

	return n;
}
