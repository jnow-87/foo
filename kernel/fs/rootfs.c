#include <arch/core.h>
#include <arch/mem.h>
#include <kernel/init.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/kmem.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/list.h>
#include <sys/math.h>


/* global variables */
fs_node_t *fs_root = 0x0;


/* local variables */
static int rootfs_id;


/* local/static prototypes */
static int open(fs_node_t *start, char const *path, f_mode_t mode);
static int close(fs_filed_t *fd);
static size_t read(fs_filed_t *fd, void *buf, size_t n);
static size_t write(fs_filed_t *fd, void *buf, size_t n);
static int fcntl(fs_filed_t *fd, int cmd, void *data);
static int rmnode(fs_node_t *start, char const *path);
static int chdir(fs_node_t *start, char const *path);

static rootfs_file_t *file_alloc(void);
static void file_free(rootfs_file_t *file);
static int file_seek(fs_filed_t *fd, seek_t *p);


/* global functions */
fs_node_t *rootfs_mkdir(char const *path, int fs_id){
	int n;
	fs_node_t *node;


	if(*path == 0)
		goto_errno(err, E_INVAL);

	DEBUG("register file system to \"%s\"\n", path);

	node = fs_root;

	while(1){
		n = fs_node_find(&node, &path);

		switch(n){
		case 0:
			/* target node already exists */
			goto_errno(err, E_INUSE);

		case -1:
		case -2:
			/* error occured */
			goto_errno(err, E_INVAL);

		default:
			/* no matching node found, try to create it */
			// if its the last part of path, create the target node with fs_id
			// otherwise create an intermediate node in rootfs
			if(path[n] == 0)	node = fs_node_alloc(node->parent, path, n, true, fs_id);
			else				node = fs_node_alloc(node->parent, path, n, true, rootfs_id);

			if(node == 0x0)
				goto err;

			path += n;

			if(*path == 0)
				return node;
			break;
		}
	}


err:
	return 0x0;
}

int rootfs_rmdir(fs_node_t *node){
	if(!list_empty(node->childs) || node->data != 0x0)
		return_errno(E_INUSE);

	DEBUG("release file system from \"%s\"\n", node->name);

	return fs_node_free(node);
}


/* local functions */
static int init(void){
	fs_ops_t ops;
	fs_node_t dummy;


	/* register rootfs */
	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = 0x0;
	ops.fcntl = fcntl;
	ops.rmnode = rmnode;
	ops.chdir = chdir;

	rootfs_id = fs_register(&ops);

	if(rootfs_id < 0)
		return errno;

	/* init fs_root */
	fs_root = fs_node_alloc(&dummy, "/", 1, true, rootfs_id);

	if(fs_root == 0x0)
		return errno;

	fs_root->parent = fs_root;

	return E_OK;
}

kernel_init(1, init);

static int open(fs_node_t *start, char const *path, f_mode_t mode){
	int n;
	fs_filed_t *fd;


	if(*path == 0)
		return_errno(E_INVAL);

	DEBUG("handle open for \"%s\"\n", path);

	while(1){
		n = fs_node_find(&start, &path);

		switch(n){
		case 0:
			/* target node found, so create file descriptor */
			fd = fs_fd_alloc(start);

			if(fd == 0x0)
				return errno;

			if(start->is_dir == false && (mode & F_APPEND))
				fd->fp = ((rootfs_file_t*)(start->data))->data_used;

			DEBUG("created file descriptor with id %d\n", fd->id);

			return fd->id;

		case -2:
			/* file system boundary reached, hence call subsequent file system handler */
			if(start->ops->open == 0x0)
				return_errno(E_NOIMP);

			DEBUG("call subsequent file system\n");

			return start->ops->open(start, path, mode);

		case -1:
			/* error occured */
			return_errno(E_INVAL);

		default:
			/* no matching node found, try to create it */
			// error if node shall not be created
			if((mode & F_CREATE) == 0)
				return_errno(E_UNAVAIL);

			// create node
			start = fs_node_alloc(start, path, n, (path[n] == '/' ? true : false), rootfs_id);

			if(start == 0x0)
				return errno;

			// allocate file
			if(start->is_dir == false){
				start->data = file_alloc();

				if(start->data == 0x0)
					goto_errno(err, E_NOMEM);
			}

			path += n;
			break;
		}
	}


err:
	fs_node_free(start);
	return errno;
}

static int close(fs_filed_t *fd){
	DEBUG("handle close for %d\n", fd->id);

	fs_fd_free(fd);
	return E_OK;
}

static size_t read(fs_filed_t *fd, void *buf, size_t n){
	size_t m;
	fs_node_t *child;
	rootfs_file_t *file;


	DEBUG("handle read for %d from fp %u, %u bytes\n", fd->id, fd->fp, n);

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

static size_t write(fs_filed_t *fd, void *buf, size_t n){
	size_t f_size;
	void *data;
	rootfs_file_t *file;


	DEBUG("handle write for %d to fp %u, %u bytes\n", fd->id, fd->fp, n);

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

static int fcntl(fs_filed_t *fd, int cmd, void *data){
	DEBUG("handle fcntl for %d\n", fd->id);

	switch(cmd){
	case F_SEEK:
		return file_seek(fd, data);

	case F_TELL:
		((seek_t*)data)->pos = fd->fp;
		return errno;

	default:
		return_errno(E_INVAL);
	}
}

static int rmnode(fs_node_t *start, char const *path){
	if(*path == 0)
		return_errno(E_INVAL);

	DEBUG("remove file system node \"%s\"\n", path);

	switch(fs_node_find(&start, &path)){
	case 0:
		/* target node found, so remove it */
		if(start->data)
			file_free(start->data);

		return fs_node_free(start);

	case -2:
		/* file system boundary reached, hence call subsequent file system handler */
		if(start->ops->rmnode == 0x0)
			return_errno(E_NOIMP);

		return start->ops->rmnode(start, path);

	case -1:
		/* part of path is not a directory */
		return_errno(E_INVAL);

	default:
		/* target node not found */
		return_errno(E_UNAVAIL);
	}
}

static int chdir(fs_node_t *start, char const *path){
	if(*path == 0)
		return_errno(E_INVAL);

	switch(fs_node_find(&start, &path)){
	case 0:
		/* target node found, set current process working directory */
		current_thread[PIR]->parent->cwd = start;
		return E_OK;

	case -2:
		/* file system boundary reached, hence call subsequent file system handler */
		if(start->ops->chdir == 0x0)
			return_errno(E_NOIMP);

		return start->ops->chdir(start, path);

	case -1:
		return_errno(E_INVAL);

	default:
		return_errno(E_UNAVAIL);
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
