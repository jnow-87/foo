#include <arch/core.h>
#include <kernel/fs.h>
#include <kernel/kmem.h>
#include <kernel/sched.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/string.h>


/* global functions */
fs_filed_t *fs_fd_alloc(fs_node_t *node){
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

void fs_fd_free(fs_filed_t *fd){
	list_rm(current_thread[PIR]->parent->fds, fd);
	fd->node->ref_cnt--;

	kfree(fd);
}

fs_node_t *fs_node_alloc(fs_node_t *parent, char const *name, size_t name_len, bool is_dir, fs_ops_t *ops){
	fs_node_t *node;


	/* check parameter */
	if(ops == 0x0 || ops->open == 0x0)
		goto_errno(err_0, E_INVAL);

	/* allocate node */
	node = kmalloc(sizeof(fs_node_t));

	if(node == 0x0)
		goto_errno(err_0, E_NOMEM);

	node->name = kmalloc(name_len + 1);

	if(node->name == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* init node attributes */
	node->ops = ops;
	node->ref_cnt = 0;
	node->is_dir = is_dir;

	strncpy(node->name, name, name_len);
	node->name[name_len] = 0;

	node->data = 0x0;

	/* add node to file system */
	node->parent = parent;
	node->childs = 0x0;

	list_add_tail(parent->childs, node);

	return node;


err_1:
	kfree(node);

err_0:
	return 0;
}

int fs_node_free(fs_node_t *node){
	fs_node_t *child;


	if(node->ref_cnt > 0)
		return_errno(E_INUSE);

	list_for_each(node->childs, child){
		if(fs_node_free(child) != E_OK)
			return errno;
	}

	list_rm(node->parent->childs, node);

	kfree(node->name);
	kfree(node);

	return E_OK;
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
int fs_node_find(fs_node_t **start, char const **path){
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
		if((*start)->is_dir == false)
			return -1;

		/* descent directory hierarchy */
		n = 0;

		while((*path)[n] != '/' && (*path)[n] != 0)
			++n;

		if(strcmp(*path, ".") == 0)			child = *start;
		else if(strcmp(*path, "..") == 0)	child = (*start)->parent;
		else								child = list_find_strn((*start)->childs, name, *path, n);

		/* no matching child found */
		if(child == 0x0)
			return n;

		*start = child;
		*path += n;

		/* skip '/' */
		while(**path == '/' && **path != 0)
			++(*path);

		/* child uses different callbacks */
		if(child->parent->ops != child->ops)
			return -2;
	}
}
