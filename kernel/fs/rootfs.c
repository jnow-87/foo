#include <arch/core.h>
#include <arch/mem.h>
#include <kernel/init.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/kmem.h>
#include <kernel/sched.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/list.h>
#include <sys/math.h>


/* global variables */
fs_node_t fs_root;


/* static variables */
static int rootfs_id;


/* local/static prototypes */
static int rootfs_open(fs_node_t *start, char const *path, f_mode_t mode);
static int rootfs_close(fs_filed_t *fd);
static size_t rootfs_read(fs_filed_t *fd, void *buf, size_t n);
static size_t rootfs_write(fs_filed_t *fd, void *buf, size_t n);
static int rootfs_fcntl(fs_filed_t *fd, int cmd, void *data);
static int rootfs_rmnode(fs_node_t *start, char const *path);
static int rootfs_chdir(fs_node_t *start, char const *path);

static fs_node_t *node_alloc(fs_node_t *parent, char const *name, size_t name_len, bool is_dir);
static int node_free(fs_node_t *node);
static int node_find(fs_node_t **start, char const **path);

static rootfs_file_t *file_alloc(void);
static void file_free(rootfs_file_t *file);
static int file_seek(fs_filed_t *fd, seek_t *p);


/* global functions */
fs_node_t *rootfs_mkdir(int fs_id, char const *path){
	int n;
	fs_node_t *node;


	if(*path == 0)
		goto_errno(err, E_INVAL);

	node = &fs_root;

	while(1){
		n = node_find(&node, &path);

		switch(n){
		case 0:
			/* target node already exists */
			// return an error if the target node is not a directory or contains childs
			if(node->is_dir == false)
				goto_errno(err, E_INVAL);

			if(!list_empty(node->childs))
				goto_errno(err, E_INUSE);

			// change file system type of target node and return it
			node->fs_id = fs_id;

			return node;

		case -1:
		case -2:
			/* error occured */
			goto_errno(err, E_INVAL);

		default:
			/* no matching node found, try to create it */
			node = node_alloc(node->parent, path, n, (path[n] == '/' ? true : false));

			if(node == 0x0)
				goto err;
			break;
		}
	}


err:
	return 0x0;
}

int rootfs_rmdir(fs_node_t *node){
	if(!list_empty(node->childs) || node->data != 0x0)
		return_errno(E_INUSE);
	return node_free(node);
}


/* local functions */
static int rootfs_init(void){
	fs_ops_t ops;


	/* register rootfs */
	ops.open = rootfs_open;
	ops.close = rootfs_close;
	ops.read = rootfs_read;
	ops.write = rootfs_write;
	ops.ioctl = 0x0;
	ops.fcntl = rootfs_fcntl;
	ops.rmnode = rootfs_rmnode;
	ops.chdir = rootfs_chdir;

	rootfs_id = fs_register(&ops);

	if(rootfs_id < 0)
		return errno;

	/* init fs_root */
	memset(&fs_root, 0x0, sizeof(fs_node_t));

	fs_root.fs_id = rootfs_id;
	fs_root.is_dir = true;

	fs_root.name = kmalloc(2);

	if(fs_root.name == 0x0)
		return_errno(E_NOMEM);

	strncpy(fs_root.name, "/", 2);

	return E_OK;
}

kernel_init(1, rootfs_init);

static int rootfs_open(fs_node_t *start, char const *path, f_mode_t mode){
	int n;
	fs_filed_t *fd;
	fs_ops_t *ops;


	if(*path == 0)
		return_errno(E_INVAL);

	while(1){
		n = node_find(&start, &path);

		switch(n){
		case 0:
			/* target node found, so create file descriptor */
			fd = fs_mkfd(start);

			if(fd == 0x0)
				return errno;

			if(start->is_dir == false && (mode & F_APPEND))
				fd->fp = ((rootfs_file_t*)(start->data))->data_used;

			return fd->id;

		case -2:
			/* file system boundary reached, hence call subsequent file system handler */
			ops = fs_get_ops(start->fs_id);

			if(ops == 0x0 || ops->open == 0x0)
				return_errno(E_NOIMP);

			return ops->open(start, path, mode);

		case -1:
			/* error occured */
			return_errno(E_INVAL);

		default:
			/* no matching node found, try to create it */
			// error if node shall not be created
			if((mode & F_CREATE) == 0)
				return_errno(E_UNAVAIL);

			// create node
			start = node_alloc(start, path, n, (path[n] == '/' ? true : false));

			if(start == 0x0)
				return errno;

			path += n;
			break;
		}
	}
}

static int rootfs_close(fs_filed_t *fd){
	fs_rmfd(fd);
	return E_OK;
}

static size_t rootfs_read(fs_filed_t *fd, void *buf, size_t n){
	size_t m;
	fs_node_t *child;
	rootfs_file_t *file;


	if(fd->node->is_dir){
		m = 0;

		/* iterate to the child indicated by the file pointer */
		list_for_each(fd->node->childs, child){
			if(m == fd->fp)
				break;

			++m;
		}

		/* no further child available */
		if(child == 0x0)
			goto_errno(err, E_END);

		/* check if buffer is large enough */
		m = strlen(child->name);

		if(m > n)
			goto_errno(err, E_LIMIT);

		/* copy child name to buffer */
		memcpy(buf, child->name, m);

		/* update file pointer */
		fd->fp++;

		return m;

	}
	else{
		file = fd->node->data;

		/* check if data is left to read */
		if(fd->fp == file->data_used)
			goto_errno(err, E_END);

		/* identify number of bytes to read and copy them to buf */
		m = MIN(n, file->data_used - fd->fp);
		memcpy(buf, file->data + fd->fp, m);

		/* update file pointer */
		fd->fp += m;

		return m;
	}


err:
	return 0;
}

static size_t rootfs_write(fs_filed_t *fd, void *buf, size_t n){
	size_t f_size;
	void *data;
	rootfs_file_t *file;


	/* write to a directory is not defined */
	if(fd->node->is_dir)
		goto_errno(err, E_INVAL);

	file = fd->node->data;

	/* adjust file size if required */
	f_size = MAX(fd->fp + n, file->data_max);

	if(f_size > file->data_max){
		f_size = MAX(f_size, 2 * file->data_max);

		data = kmalloc(f_size);

		if(data == 0x0)
			goto_errno(err, E_NOMEM);

		memcpy(data, file->data, file->data_used);

		kfree(file->data);
		file->data = data;
	}

	/* update file content */
	memcpy(file->data + fd->fp, buf, n);

	fd->fp += n;
	file->data_used = MAX(fd->fp, file->data_used);

	return n;


err:
	return 0;
}

static int rootfs_fcntl(fs_filed_t *fd, int cmd, void *data){
	seek_t seek_p;


	switch(cmd){
	case F_SEEK:
		(void)copy_from_user(&seek_p, data, sizeof(seek_t), current_thread[PIR]->parent);

		return file_seek(fd, &seek_p);

	case F_TELL:
		seek_p.pos = fd->fp;
		(void)copy_to_user(data, &seek_p, sizeof(seek_t), current_thread[PIR]->parent);

		return errno;

	default:
		return_errno(E_INVAL);
	}
}

static int rootfs_rmnode(fs_node_t *start, char const *path){
	fs_ops_t *ops;


	if(*path == 0)
		return_errno(E_INVAL);

	switch(node_find(&start, &path)){
	case 0:
		/* target node found, so remove it */
		return node_free(start);

	case -2:
		/* file system boundary reached, hence call subsequent file system handler */
		ops = fs_get_ops(start->fs_id);

		if(ops == 0x0 || ops->rmnode == 0x0)
			return_errno(E_NOIMP);

		return ops->rmnode(start, path);

	case -1:
		/* part of path is not a directory */
		return_errno(E_INVAL);

	default:
		/* target node not found */
		return_errno(E_UNAVAIL);
	}
}

static int rootfs_chdir(fs_node_t *start, char const *path){
	fs_ops_t *ops;


	if(*path == 0)
		return_errno(E_INVAL);

	switch(node_find(&start, &path)){
	case 0:
		/* target node found, set current process working directory */
		current_thread[PIR]->parent->cwd = start;
		return E_OK;

	case -2:
		/* file system boundary reached, hence call subsequent file system handler */
		ops = fs_get_ops(start->fs_id);

		if(ops == 0x0 || ops->chdir == 0x0)
			return_errno(E_NOIMP);

		return ops->chdir(start, path);

	case -1:
		return_errno(E_INVAL);

	default:
		return_errno(E_UNAVAIL);
	}
}

static fs_node_t *node_alloc(fs_node_t *parent, char const *name, size_t name_len, bool is_dir){
	fs_node_t *node,
			  *child;


	/* allocate node */
	node = kmalloc(sizeof(fs_node_t));

	if(node == 0x0)
		goto_errno(err_0, E_NOMEM);

	node->name = kmalloc(name_len + 1);

	if(node->name == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* init node attributes */
	node->fs_id = rootfs_id;
	node->ref_cnt = 0;
	node->is_dir = is_dir;

	strncpy(node->name, name, name_len);
	node->name[name_len] = 0;

	node->data = 0x0;

	if(is_dir == false && strcmp(name, ".") != 0 && strcmp(name, "..") != 0){
		node->data = file_alloc();

		if(node->data == 0x0)
			goto_errno(err_2, E_NOMEM);
	}

	/* add node to file system */
	node->parent = parent;
	node->childs = 0x0;

	// add '.' and '..' childs for directories
	if(is_dir){
		child = node_alloc(node, ".", 1, false);

		if(child == 0x0)
			goto err_2;

		child = node_alloc(node, "..", 2, false);

		if(child == 0x0)
			goto err_3;
	}

	list_add_tail(parent->childs, node);

	return node;


err_3:
	node_free(node->childs);

err_2:
	kfree(node->name);

err_1:
	kfree(node);

err_0:
	return 0;
}

static int node_free(fs_node_t *node){
	fs_node_t *child;


	if(node->ref_cnt > 0)
		return_errno(E_INUSE);

	list_for_each(node->childs, child){
		if(node_free(child) != E_OK)
			return errno;
	}

	list_rm(node->parent->childs, node);

	file_free(node->data);
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
 *			-2	start is of different file system type than rootfs_id
 *
 * \post	start contains a pointer to the last valid node.
 * 			if the return value is 0, start contains the target node.
 * 			if the return value is 1 the file system id of the current
 * 			node is different from rootfs_id, hence a subsequent file
 * 			system needs to be called to continue the search.
 *
 * \post	path points to the next part of the path that needs to be
 * 			evaluated. if start is the target node, path points to 0x0.
 */
static int node_find(fs_node_t **start, char const **path){
	size_t n;
	fs_node_t *child;


	while(1){
		/* skip leading '/' */
		while(**path == '/' && **path != 0)
			++(*path);

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

		child = list_find_strn((*start)->childs, name, *path, n);

		/* no matching child found */
		if(child == 0x0)
			return n;

		*start = child;
		*path += n;

		/* child is of different file system id */
		if((*start)->fs_id != rootfs_id)
			return -2;
	}
}

static rootfs_file_t *file_alloc(void){
	rootfs_file_t *f;


	/* allocate file */
	f = kmalloc(sizeof(rootfs_file_t));

	if(f == 0x0)
		goto_errno(err_0, E_NOMEM);

	f->data = kmalloc(CONFIG_ROOTFS_INIT_FILE_SIZE);

	if(f->data == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* init file attributes */
	f->data_max = CONFIG_ROOTFS_INIT_FILE_SIZE;
	f->data_used = 0;

	return f;


err_1:
	kfree(f);

err_0:
	return 0;
}

static void file_free(rootfs_file_t *file){
	if(file == 0x0)
		return;

	kfree(file->data);
	kfree(file);
}

static int file_seek(fs_filed_t *fd, seek_t *p){
	size_t whence;
	rootfs_file_t *file;


	file = (rootfs_file_t*)fd->node->data;

	if(p->whence == SEEK_SET)		whence = 0;
	else if(p->whence == SEEK_CUR)	whence = fd->fp;
	else if(p->whence == SEEK_END)	whence = file->data_used;

	if(whence + p->offset > file->data_used)
		goto_errno(k_ok, E_LIMIT);

	fd->fp = whence + p->offset;


k_ok:
	return E_OK;
}
