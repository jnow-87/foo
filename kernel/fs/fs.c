/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/core.h>
#include <kernel/fs.h>
#include <kernel/memory.h>
#include <kernel/sched.h>
#include <kernel/ksignal.h>
#include <sys/limits.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/stat.h>
#include <sys/mutex.h>
#include <sys/string.h>


/* macros */
#define FMODE_DEFAULT	(O_NONBLOCK)


/* local/static prototypes */
static void rw_lock(void);
static void rw_unlock(void);

static int fs_node_find_unsafe(fs_node_t **start, char const **path);


/* static variables */
static fs_t *fs_lst = 0x0;
static mutex_t fs_ro_mtx = NESTED_MUTEX_INITIALISER(),
			   fs_rw_mtx = MUTEX_INITIALISER();


/* global functions */
int fs_register(fs_ops_t *ops){
	int id = 0;
	fs_t *fs;


	rw_lock();

	if(!list_empty(fs_lst))
		id = list_last(fs_lst)->id + 1;

	if(id < 0)
		goto_errno(err, E_LIMIT);

	if(ops == 0x0 || ops->open == 0x0 || ops->close == 0x0)
		goto_errno(err, E_INVAL);

	fs = kmalloc(sizeof(fs_t));

	if(fs == 0x0)
		goto err;

	fs->id = id;
	fs->ops = *ops;

	list_add_tail(fs_lst, fs);

	rw_unlock();

	return fs->id;


err:
	rw_unlock();

	return -errno;
}

void fs_release(int id){
	fs_t *fs;


	rw_lock();

	fs = list_find(fs_lst, id, id);

	if(fs != 0x0){
		list_rm(fs_lst, fs);
		kfree(fs);
	}

	rw_unlock();
}

void fs_lock(void){
	mutex_lock(&fs_ro_mtx);
}

void fs_unlock(void){
	mutex_unlock(&fs_ro_mtx);
}

fs_filed_t *fs_fd_alloc(fs_node_t *node, process_t *this_p, f_mode_t mode){
	int id = 0;
	fs_filed_t *fd;


	mutex_lock(&this_p->mtx);
	rw_lock();

	/* acquire descriptor id */
	if(!list_empty(this_p->fds))
		id = list_last(this_p->fds)->id + 1;

	if(id < 0)
		goto_errno(err_0, E_LIMIT);

	/* allocate file descriptor */
	fd = kmalloc(sizeof(fs_filed_t));

	if(fd == 0x0)
		goto err_0;

	fd->id = id;
	fd->node = node;
	fd->fp = 0;
	fd->mode = mode;
	mutex_init(&fd->mtx, MTX_NONE);

	list_add_tail(this_p->fds, fd);
	node->ref_cnt++;

	rw_unlock();
	mutex_unlock(&this_p->mtx);

	return fd;


err_0:
	rw_unlock();
	mutex_unlock(&this_p->mtx);

	return 0x0;
}

int fs_fd_dup(fs_filed_t *old_fd, int id, struct process_t *this_p){
	fs_filed_t *fd,
			   *e;


	mutex_lock(&this_p->mtx);
	rw_lock();

	/* acquire descriptor id if no valid is given */
	if(id < 0){
		if(!list_empty(this_p->fds))
			id = list_last(this_p->fds)->id + 1;

		if(id < 0)
			goto_errno(err_0, E_LIMIT);
	}

	/* allocate file descriptor */
	fd = kmalloc(sizeof(fs_filed_t));

	if(fd == 0x0)
		goto err_0;

	fd->id = id;
	fd->node = old_fd->node;
	fd->fp = old_fd->fp;
	fd->mode = old_fd->mode;
	mutex_init(&fd->mtx, MTX_NONE);

	fd->node->ref_cnt++;

	// add fd to process (sorted)
	list_for_each(this_p->fds, e){
		if(e->id > id){
			list_add_in(fd, e->prev, e);
			break;
		}
	}

	if(e == 0x0)
		list_add_tail(this_p->fds, fd);

	rw_unlock();
	mutex_unlock(&this_p->mtx);

	return id;


err_0:
	rw_unlock();
	mutex_unlock(&this_p->mtx);

	return -errno;
}

void fs_fd_free(fs_filed_t *fd, process_t *this_p){
	list_rm_safe(this_p->fds, fd, &this_p->mtx);

	rw_lock();
	fd->node->ref_cnt--;
	rw_unlock();

	kfree(fd);
}

fs_filed_t *fs_fd_acquire(int id, struct process_t *this_p){
	fs_filed_t *fd;


	mutex_lock(&this_p->mtx);

	fd = list_find(this_p->fds, id, id);

	if(fd != 0x0)
		mutex_lock(&fd->mtx);

	mutex_unlock(&this_p->mtx);

	return fd;
}

void fs_fd_release(fs_filed_t *fd){
	mutex_unlock(&fd->mtx);
}

int fs_fd_wait(fs_filed_t *fd, ksignal_t *sig, mutex_t *mtx){
	fs_node_t *node;


	node = fd->node;

	if(fd->mode & O_NONBLOCK)
		return -1;

	if(node->timeout_us > 0)	ksignal_timedwait(sig, mtx, node->timeout_us);
	else						ksignal_wait(sig, mtx);

	return 0;
}

fs_node_t *fs_node_create(fs_node_t *parent, char const *name, size_t name_len, file_type_t type, void *payload, int fs_id){
	fs_t *fs;
	fs_node_t *node;


	fs_lock();

	if(name_len + 1 > NAME_MAX)
		goto_errno(err_0, E_LIMIT);

	/* identify file system */
	list_for_each(fs_lst, fs){
		if(fs->id == fs_id)
			break;
	}

	if(fs == 0x0)
		goto_errno(err_0, E_INVAL);

	/* allocate node */
	node = kmalloc(sizeof(fs_node_t) + name_len + 1);

	if(node == 0x0)
		goto err_0;

	/* init node attributes */
	node->fs_id = fs_id;
	node->ops = &fs->ops;
	node->ref_cnt = 0;
	node->type = type;

	strncpy(node->name, name, name_len);
	node->name[name_len] = 0;

	node->parent = parent;
	node->childs = 0x0;
	node->payload = payload;

	mutex_init(&node->mtx, MTX_NOINT);
	ksignal_init(&node->datain_sig);
	node->timeout_us = CONFIG_FS_OP_TIMEOUT_US;

	/* add node to file system */
	rw_lock();
	list_add_tail(parent->childs, node);
	rw_unlock();

	/* add '.' and '..' nodes for directories */
	if(type == FT_DIR){
		if(fs_node_create(node, ".", 1, FT_LNK, node, fs_id) == 0x0)
			goto err_1;

		if(fs_node_create(node, "..", 2, FT_LNK, parent, parent->fs_id) == 0x0)
			goto err_1;
	}

	fs_unlock();

	return node;


err_1:
	fs_node_destroy(node);

err_0:
	fs_unlock();

	return 0x0;
}

int fs_node_destroy(fs_node_t *node){
	fs_node_t *child;


	fs_lock();

	if(node->ref_cnt > 0)
		goto_errno(err, E_INUSE);

	list_for_each(node->childs, child){
		if(fs_node_destroy(child) != 0)
			goto err;
	}

	rw_lock();
	list_rm(node->parent->childs, node);
	rw_unlock();

	kfree(node);

	fs_unlock();

	return 0;


err:
	fs_unlock();

	return -errno;
}

int fs_node_find(fs_node_t **start, char const **path){
	int r;


	fs_lock();
	r = fs_node_find_unsafe(start, path);
	fs_unlock();

	return r;
}


/* local functions */
static void rw_lock(void){
	mutex_lock(&fs_rw_mtx);
	mutex_lock(&fs_ro_mtx);
}

static void rw_unlock(void){
	mutex_unlock(&fs_ro_mtx);
	mutex_unlock(&fs_rw_mtx);
}

/**
 * \brief	try to find the node specified in path
 *
 * \param	start	node to use as search root
 * \param	path	path to search for
 *
 * \return	0	target node found
 * 			>0	no matching node found for the name of the next n characters of path
 * 			-1	a part of path is not a directory
 *			-2	file system callbacks changed, i.e. the file system type changed
 *
 * \post	start contains a pointer to the last valid node.
 * \post	path points to the next part of the path that needs to be
 * 			evaluated. if start is the target node, path points to 0x0.
 */
static int fs_node_find_unsafe(fs_node_t **start, char const **path){
	size_t n;
	fs_node_t *child;


	/* skip leading '/' */
	while(**path == '/' && **path != 0)
		++(*path);

	while(1){
		/* end of path reached, hence start is the desired node */
		if(**path == 0)
			return 0;

		/* return if start is no directory */
		if((*start)->type != FT_DIR)
			return -1;

		/* descent directory hierarchy */
		n = 0;

		while((*path)[n] != '/' && (*path)[n] != 0)
			++n;

		list_for_each((*start)->childs, child){
			if(strncmp(child->name, *path, n) == 0 && (child->name[n] == '/' || child->name[n] == '\0'))
				break;
		}

		/* no matching child found */
		if(child == 0x0)
			return n;

		/* update arguments */
		*path += n;
		*start = child;

		if(child->type == FT_LNK)
			*start = child->payload;

		// skip '/'
		while(**path == '/' && **path != 0)
			++(*path);

		/* next iteration uses different callbacks */
		if(child->parent->ops != (*start)->ops)
			return -2;
	}
}
