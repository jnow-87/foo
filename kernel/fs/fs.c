#include <arch/core.h>
#include <kernel/fs.h>
#include <kernel/kmem.h>
#include <kernel/sched.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/string.h>


/* static variables */
static fs_t *fs_lst = 0x0;


/* global functions */
int fs_register(fs_ops_t *ops){
	fs_t *fs;
	unsigned int id;


	if(ops == 0 || ops->open == 0x0)
		return_errno(E_INVAL);

	id = 0;

	if(!list_empty(fs_lst))
		id = list_last(fs_lst)->id + 1;

	if(id < 0)
		return_errno(E_LIMIT);

	fs = kmalloc(sizeof(fs_t));

	if(fs == 0x0)
		return_errno(E_NOMEM);

	fs->id = id;
	fs->ops = kmalloc(sizeof(fs_ops_t));

	if(fs->ops == 0)
		goto_errno(err, E_NOMEM);

	memcpy(fs->ops, ops, sizeof(fs_ops_t));

	list_add_tail(fs_lst, fs);

	return fs->id;


err:
	kfree(fs);

	return errno;
}

int fs_release(int fs_id){
	fs_t *fs;


	fs = list_find(fs_lst, id, fs_id);

	if(fs == 0x0)
		return_errno(E_INVAL);

	list_rm(fs_lst, fs);
	kfree(fs->ops);
	kfree(fs);

	return E_OK;
}

fs_ops_t *fs_get_ops(int fs_id){
	fs_t *fs;


	fs = list_find(fs_lst, id, fs_id);

	if(fs != 0x0)
		return fs->ops;

	errno = E_INVAL;
	return 0;
}

fs_filed_t *fs_mkfd(fs_node_t *node){
	int id;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = current_thread[PIR]->parent;

	/* acquire descriptor id */
	id = 0;

	if(!list_empty(this_p->fds))
		id = list_last(this_p->fds)->id + 1;

	if(id == (unsigned int)(-1))
		goto_errno(err, E_LIMIT);

	/* allocate file descriptor */
	fd = kmalloc(sizeof(fs_filed_t));

	if(fd == 0x0)
		goto_errno(err, E_NOMEM);

	fd->id = id;
	fd->node = node;
	fd->fp = 0;

	list_add_tail(this_p->fds, fd);
	node->ref_cnt++;

	return fd;


err:
	return 0x0;
}

void fs_rmfd(fs_filed_t *fd){
	list_rm(current_thread[PIR]->parent->fds, fd);
	fd->node->ref_cnt--;

	kfree(fd);
}
