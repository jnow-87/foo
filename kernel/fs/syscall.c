/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/core.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/sched.h>
#include <kernel/syscall.h>
#include <kernel/rootfs.h>
#include <kernel/ksignal.h>
#include <kernel/kprintf.h>
#include <sys/string.h>
#include <sys/fcntl.h>
#include <sys/list.h>



/* local/static prototypes */
// syscall handler
static int sc_hdlr_open(void *param);
static int sc_hdlr_dup(void *param);
static int sc_hdlr_close(void *param);
static int sc_hdlr_read(void *param);
static int sc_hdlr_write(void *param);
static int sc_hdlr_ioctl(void *param);
static int sc_hdlr_fcntl(void *param);
static int sc_hdlr_rmnode(void *param);
static int sc_hdlr_chdir(void *param);

// actual operations
static int fcntl(fs_filed_t *fd, int cmd, void *data, process_t *this_p);


/* local functions */
static int init(void){
	int r;


	r = E_OK;

	r |= sc_register(SC_OPEN, sc_hdlr_open);
	r |= sc_register(SC_DUP, sc_hdlr_dup);
	r |= sc_register(SC_CLOSE, sc_hdlr_close);
	r |= sc_register(SC_READ, sc_hdlr_read);
	r |= sc_register(SC_WRITE, sc_hdlr_write);
	r |= sc_register(SC_IOCTL, sc_hdlr_ioctl);
	r |= sc_register(SC_FCNTL, sc_hdlr_fcntl);
	r |= sc_register(SC_RMNODE, sc_hdlr_rmnode);
	r |= sc_register(SC_CHDIR, sc_hdlr_chdir);

	return r;
}

kernel_init(0, init);

static int sc_hdlr_open(void *_p){
	char path[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_node_t *root;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	copy_from_user(path, p->data, p->data_len, this_p);

	/* identify file system and call its open callback */
	mutex_lock(&this_p->mtx);
	root = (path[0] == '/') ? (fs_root) : this_p->cwd;
	mutex_unlock(&this_p->mtx);

	if(root->ops->open == 0x0)
		return_errno(E_NOIMP);

	DEBUG("path \"%s\", mode %#x\n", path, p->mode);
	p->fd = root->ops->open(root, path, p->mode, this_p);
	DEBUG("created fd with id %d, \"%s\"\n", p->fd, strerror(errno));

	return -errno;
}

static int sc_hdlr_dup(void *_p){
	int old_id;
	sc_fs_t *p;
	fs_filed_t *old_fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initiali */
	p = (sc_fs_t*)_p;
	copy_from_user(&old_id, p->data, sizeof(int), this_p);
	old_fd = fs_fd_acquire(old_id, this_p);

	DEBUG("oldfd %d%s, newfd %d\n", old_id, (old_fd == 0x0 ? " (invalid)" : ""), p->fd);

	// exit if oldfd does not exist or old and new fd are the same
	if(old_fd == 0x0 || old_fd->id == p->fd)
		return_errno(E_INVAL);

	/* close the desired fd if one is given */
	if(p->fd >= 0)
		sc_hdlr_close(_p);

	// E_INVAL is expected in case the desired fd
	// was not open before
	if(errno && errno != E_INVAL)
		return -errno;

	errno = E_OK;

	/* duplicate old fd */
	p->fd = fs_fd_dup(old_fd, p->fd, this_p);

	fs_fd_release(old_fd);

	DEBUG("created fd with id %d\n", p->fd);

	return E_OK;
}

static int sc_hdlr_close(void *_p){
	sc_fs_t *p;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* handle close */
	// NOTE fs_fd_release must not be called, since
	// 		close has already deleted the decriptor
	return fd->node->ops->close(fd, this_p);
}

static int sc_hdlr_read(void *_p){
	ssize_t r;
	char buf[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* call read callback if implemented */
	if(node->ops->read == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("offset %d, max-len %u\n", fd->fp, p->data_len);

	mutex_lock(&node->mtx);

	while(1){
		r = node->ops->read(fd, buf, p->data_len);

		if(r || errno || (fd->mode & O_NONBLOCK))
			break;

		ksignal_wait_mtx(&node->datain_sig, &node->mtx);
	}

	p->data_len = r;

	mutex_unlock(&node->mtx);

	DEBUG("read %d bytes, \"%s\"\n", r, strerror(errno));

	// avoid communicating end of resource to user space
	if(errno == E_END)
		errno = E_OK;

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, buf, p->data_len, this_p);

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_write(void *_p){
	sc_fs_t *p;
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;
	char buf[((sc_fs_t*)_p)->data_len];


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* handle write */
	if(node->ops->write == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("offset %d, len %u\n", fd->fp, p->data_len);

	copy_from_user(buf, p->data, p->data_len, this_p);

	mutex_lock(&node->mtx);
	p->data_len = node->ops->write(fd, buf, p->data_len);
	mutex_unlock(&node->mtx);

	DEBUG("written %d bytes, \"%s\"\n", p->data_len, strerror(errno));

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_ioctl(void *_p){
	char data[((sc_fs_t*)(_p))->data_len];
	int r;
	sc_fs_t *p;
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* call ioctl callback if implemented */
	r = -E_NOIMP;

	if(node->ops->ioctl == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("cmd %d\n", p->cmd);

	copy_from_user(data, p->data, p->data_len, this_p);

	mutex_lock(&node->mtx);
	r = node->ops->ioctl(fd, p->cmd, data);
	mutex_unlock(&node->mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);

end:
	fs_fd_release(fd);

	return r;
}

static int sc_hdlr_fcntl(void *_p){
	char data[((sc_fs_t*)(_p))->data_len];
	int r;
	sc_fs_t *p;
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* call fcntl callback if implemented */
	DEBUG("cmd %d\n", p->cmd);

	copy_from_user(data, p->data, p->data_len, this_p);

	mutex_lock(&node->mtx);

	r = fcntl(fd, p->cmd, data, this_p);

	if(r == -E_NOIMP){
		if(node->ops->fcntl != 0x0){
			errno = E_OK;
			r = node->ops->fcntl(fd, p->cmd, data);
		}
	}

	mutex_unlock(&node->mtx);

	/* update user space */
	if(r == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);

	fs_fd_release(fd);

	return r;
}

static int sc_hdlr_rmnode(void *_p){
	char path[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_node_t *root;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	copy_from_user(path, p->data, p->data_len, this_p);

	DEBUG("%s\n", path);

	/* identify file system and call its rmnode callback */
	mutex_lock(&this_p->mtx);
	root = (path[0] == '/') ? (fs_root) : this_p->cwd;
	mutex_unlock(&this_p->mtx);

	if(root->ops->node_rm == 0x0)
		return_errno(E_NOIMP);
	return root->ops->node_rm(root, path);
}

static int sc_hdlr_chdir(void *_p){
	char path[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_node_t *root;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	copy_from_user(path, p->data, p->data_len, this_p);

	DEBUG("sc chdir: %s\n", path);

	/* identify file system and call its findnode callback */
	fs_lock();
	mutex_lock(&this_p->mtx);

	root = (path[0] == '/') ? (fs_root) : this_p->cwd;

	if(root->ops->node_find == 0x0)
		goto_errno(end, E_NOIMP);

	root = root->ops->node_find(root, path);

	if(root == 0x0)
		goto end;

	if(root->type != FT_DIR)
		goto_errno(end, E_INVAL);

	// update current working directory
	this_p->cwd->ref_cnt--;
	this_p->cwd = root;
	root->ref_cnt++;

end:
	mutex_unlock(&this_p->mtx);
	fs_unlock();

	DEBUG("new cwd \"%s\", \"%s\"\n", this_p->cwd->name, strerror(errno));

	return -errno;
}

static int fcntl(fs_filed_t *fd, int cmd, void *data, process_t *this_p){
	f_mode_t *mode;


	mode = (f_mode_t*)data;

	/* handle file descriptor mode commands */
	switch(cmd){
	case F_MODE_GET:
		*mode = fd->mode;
		break;

	case F_MODE_SET:
		fd->mode = (*mode & ~fd->mode_mask) | (fd->mode & fd->mode_mask);

		if(fd->mode != *mode)
			return_errno(E_NOSUP);
		break;

	default:
		return_errno(E_NOIMP);
	}

	return E_OK;
}
