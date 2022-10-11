/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/core.h>
#include <kernel/init.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/memory.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <kernel/ksignal.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/list.h>
#include <sys/math.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/dirent.h>


/* global variables */
fs_node_t *fs_root = 0x0;


/* local variables */
static int rootfs_id = 0;


/* local/static prototypes */
static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p);
static int open_unsafe(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p);
static int close(fs_filed_t *fd, process_t *this_p);
static size_t read(fs_filed_t *fd, void *buf, size_t n);
static size_t write(fs_filed_t *fd, void *buf, size_t n);
static int fcntl(fs_filed_t *fd, int cmd, void *arg);
static int node_rm(fs_node_t *start, char const *path);
static int node_rm_unsafe(fs_node_t *start, char const *path);
static fs_node_t *node_find(fs_node_t *start, char const *path);

static rootfs_file_t *file_alloc(void);
static void file_free(rootfs_file_t *file);
static int file_seek(fs_filed_t *fd, seek_t *p);


/* global functions */
fs_node_t *rootfs_mkdir(char const *path, int fs_id){
	int n;
	fs_node_t *node;


	if(*path == 0)
		goto_errno(err, E_INVAL);

	INFO("register file system to \"%s\"\n", path);

	fs_lock();

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
			if(path[n] == 0)	node = fs_node_create(node->parent, path, n, FT_DIR, 0x0, fs_id);
			else				node = fs_node_create(node->parent, path, n, FT_DIR, 0x0, rootfs_id);

			if(node == 0x0)
				goto err;

			path += n;

			if(*path == 0){
				fs_unlock();

				return node;
			}

			break;
		}
	}


err:
	fs_unlock();

	return 0x0;
}

int rootfs_rmdir(fs_node_t *node){
	int r;


	fs_lock();

	if(!list_empty(node->childs) || node->payload != 0x0)
		goto_errno(err, E_INUSE);

	INFO("release file system from \"%s\"\n", node->name);
	r = fs_node_destroy(node);

	fs_unlock();

	return r;


err:
	fs_unlock();

	return -errno;
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
	ops.mmap = 0x0;
	ops.node_rm = node_rm;
	ops.node_find = node_find;

	rootfs_id = fs_register(&ops);

	if(rootfs_id < 0)
		goto err_0;

	/* init fs_root */
	memset(&dummy, 0, sizeof(dummy));

	fs_root = fs_node_create(&dummy, "/", 1, FT_DIR, 0x0, rootfs_id);
	fs_node_destroy(fs_root->childs->next);		// remove the ".." child

	if(fs_root == 0x0)
		goto err_1;

	fs_root->parent = fs_root;

	return 0;


err_1:
	fs_release(rootfs_id);

err_0:
	return -errno;
}

kernel_init(1, init);

static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p){
	int r;


	fs_lock();
	r = open_unsafe(start, path, mode, this_p);
	fs_unlock();

	return r;
}

static int open_unsafe(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p){
	int n;
	fs_filed_t *fd;
	rootfs_file_t *file;
	file_type_t type;


	while(1){
		n = fs_node_find(&start, &path);

		switch(n){
		case 0:
			/* target node found, so create file descriptor */
			fd = fs_fd_alloc(start, this_p, mode);

			if(fd == 0x0)
				return -errno;

			if(start->type == FT_REG && (mode & O_APPEND))
				fd->fp = ((rootfs_file_t*)(start->payload))->size;

			return fd->id;

		case -2:
			/* file system boundary reached, hence call subsequent file system handler */
			if(start->ops->open == 0x0)
				return_errno(E_NOIMP);

			return start->ops->open(start, path, mode, this_p);

		case -1:
			/* error occured */
			return_errno(E_INVAL);

		default:
			type = (path[n] == '/') ? FT_DIR : FT_REG;

			/* no matching node found, try to create it */
			// error if node shall not be created
			if((mode & O_CREAT) == 0)
				return_errno(E_UNAVAIL);

			// allocate file
			file = 0x0;

			if(type == FT_REG){
				file = file_alloc();

				if(file == 0x0)
					return -errno;
			}

			// create node
			start = fs_node_create(start, path, n, type, file, rootfs_id);

			if(start == 0x0)
				goto err;

			path += n;
			break;
		}
	}


err:
	file_free(file);

	return -errno;
}

static int close(fs_filed_t *fd, process_t *this_p){
	fs_fd_free(fd, this_p);

	return 0;
}

static size_t read(fs_filed_t *fd, void *buf, size_t n){
	size_t m;
	fs_node_t *child;
	rootfs_file_t *file;
	dir_ent_t *dir;


	if(fd->node->type == FT_DIR){
		if(n != sizeof(dir_ent_t))
			goto_errno(err, E_INVAL);

		m = 0;
		dir = (dir_ent_t*)buf;

		/* iterate to the child indicated by the file pointer */
		list_for_each(fd->node->childs, child){
			if(m == fd->fp)
				break;

			++m;
		}

		/* no further child available */
		if(child == 0x0)
			goto_errno(err, E_END);

		/* copy child name to buffer */
		dir->type = child->type;

		// don't show "." and ".." as type FT_LNK
		if(strcmp(child->name, ".") == 0 || strcmp(child->name, "..") == 0)
			dir->type = FT_DIR;

		strcpy(dir->name, child->name);

		/* update file pointer */
		fd->fp++;
		m = sizeof(dir_ent_t);
	}
	else{
		file = fd->node->payload;

		/* check if data is left to read */
		if(fd->fp == file->size)
			goto_errno(err, E_END);

		/* identify number of bytes to read and copy them to buf */
		m = MIN(n, file->size - fd->fp);
		memcpy(buf, file->content + fd->fp, m);

		/* update file pointer */
		fd->fp += m;
	}

	return m;


err:
	return 0;
}

static size_t write(fs_filed_t *fd, void *buf, size_t n){
	size_t f_size;
	void *content;
	rootfs_file_t *file;


	/* write to a directory is not defined */
	if(fd->node->type == FT_DIR)
		goto_errno(err, E_INVAL);

	file = (rootfs_file_t*)fd->node->payload;

	/* adjust file size if required */
	f_size = MAX(fd->fp + n, file->capacity);

	if(f_size > file->capacity){
		f_size = MAX(f_size, 2 * file->capacity);

		content = kmalloc(f_size);

		if(content == 0x0)
			goto err;

		memcpy(content, file->content, file->size);

		kfree(file->content);
		file->content = content;
	}

	/* update file content */
	memcpy(file->content + fd->fp, buf, n);

	fd->fp += n;
	file->size = MAX(fd->fp, file->size);

	ksignal_send(&fd->node->datain_sig);

	return n;


err:
	return 0;
}

static int fcntl(fs_filed_t *fd, int cmd, void *arg){
	fs_node_t *node;
	stat_t *stat;


	switch(cmd){
	case F_SEEK:
		return file_seek(fd, arg);

	case F_TELL:
		((seek_t*)arg)->pos = fd->fp;
		return -errno;

	case F_STAT:
		stat = (stat_t*)arg;

		if(fd->node->type == FT_DIR){
			stat->size = 0;

			list_for_each(fd->node->childs, node)
				stat->size++;
		}
		else
			stat->size = ((rootfs_file_t*)fd->node->payload)->size;

		stat->type = fd->node->type;

		return 0;

	default:
		return_errno(E_INVAL);
	}
}

static int node_rm(fs_node_t *start, char const *path){
	int r;


	fs_lock();
	r = node_rm_unsafe(start, path);
	fs_unlock();

	return r;
}

static int node_rm_unsafe(fs_node_t *start, char const *path){
	if(*path == 0)
		return_errno(E_INVAL);

	switch(fs_node_find(&start, &path)){
	case 0:
		/* target node found, so remove it */
		if(start->payload)
			file_free(start->payload);

		return fs_node_destroy(start);

	case -2:
		/* file system boundary reached, hence call subsequent file system handler */
		if(start->ops->node_rm == 0x0)
			return_errno(E_NOIMP);

		return start->ops->node_rm(start, path);

	case -1:
		/* part of path is not a directory */
		return_errno(E_INVAL);

	default:
		/* target node not found */
		return_errno(E_UNAVAIL);
	}
}

static fs_node_t *node_find(fs_node_t *start, char const *path){
	if(*path == 0)
		goto_errno(err, E_INVAL);

	switch(fs_node_find(&start, &path)){
	case 0:
		/* target node found, set current process working directory */
		return start;

	case -2:
		/* file system boundary reached, hence call subsequent file system handler */
		if(start->ops->node_find == 0x0)
			goto_errno(err, E_NOIMP);

		return start->ops->node_find(start, path);

	case -1:
		goto_errno(err, E_INVAL);

	default:
		goto_errno(err, E_UNAVAIL);
	}


err:
	return 0x0;
}

static rootfs_file_t *file_alloc(void){
	rootfs_file_t *file;


	/* allocate file */
	file = kmalloc(sizeof(rootfs_file_t));

	if(file == 0x0)
		goto err_0;

	file->content = kmalloc(CONFIG_ROOTFS_INIT_FILE_SIZE);

	if(file->content == 0x0)
		goto err_1;

	/* init file attributes */
	file->capacity = CONFIG_ROOTFS_INIT_FILE_SIZE;
	file->size = 0;

	return file;


err_1:
	kfree(file);

err_0:
	return 0;
}

static void file_free(rootfs_file_t *file){
	if(file == 0x0)
		return;

	kfree(file->content);
	kfree(file);
}

static int file_seek(fs_filed_t *fd, seek_t *p){
	rootfs_file_t *file = (rootfs_file_t*)fd->node->payload;
	size_t whence;


	if(p->whence == SEEK_SET)		whence = 0;
	else if(p->whence == SEEK_CUR)	whence = fd->fp;
	else if(p->whence == SEEK_END)	whence = file->size;
	else							return_errno(E_NOIMP);

	if(whence + p->offset > file->size)
		goto_errno(k_ok, E_LIMIT);

	fd->fp = whence + p->offset;


k_ok:
	return 0;
}
