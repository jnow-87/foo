/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/memory.h>
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
static int sc_hdlr_mmap(void *param);
static int sc_hdlr_rmnode(void *param);
static int sc_hdlr_chdir(void *param);

// actual operations
static int fcntl(fs_filed_t *fd, int cmd, void *arg, process_t *this_p);


/* local functions */
static int init(void){
	int r = 0;


	r |= sc_register(SC_OPEN, sc_hdlr_open);
	r |= sc_register(SC_DUP, sc_hdlr_dup);
	r |= sc_register(SC_CLOSE, sc_hdlr_close);
	r |= sc_register(SC_READ, sc_hdlr_read);
	r |= sc_register(SC_WRITE, sc_hdlr_write);
	r |= sc_register(SC_IOCTL, sc_hdlr_ioctl);
	r |= sc_register(SC_FCNTL, sc_hdlr_fcntl);
	r |= sc_register(SC_MMAP, sc_hdlr_mmap);
	r |= sc_register(SC_RMNODE, sc_hdlr_rmnode);
	r |= sc_register(SC_CHDIR, sc_hdlr_chdir);

	return r;
}

kernel_init(0, init);

static int sc_hdlr_open(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	char path[p->payload_len];
	fs_node_t *root;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;

	if(copy_from_user(path, p->payload, p->payload_len, this_p) != 0)
		return -errno;

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

static int sc_hdlr_dup(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	int old_id;
	fs_filed_t *old_fd;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;

	if(copy_from_user(&old_id, p->payload, sizeof(int), this_p) != 0)
		return -errno;

	old_fd = fs_fd_acquire(old_id, this_p);

	DEBUG("oldfd %d%s, newfd %d\n", old_id, (old_fd == 0x0 ? " (invalid)" : ""), p->fd);

	// exit if oldfd does not exist or old and new fd are the same
	if(old_fd == 0x0 || old_fd->id == p->fd)
		return_errno(E_INVAL);

	/* close the desired fd if one is given */
	if(p->fd >= 0)
		sc_hdlr_close(param);

	// E_INVAL is expected in case the desired fd
	// was not open before
	if(errno && errno != E_INVAL)
		return -errno;

	reset_errno();

	/* duplicate old fd */
	p->fd = fs_fd_dup(old_fd, p->fd, this_p);

	fs_fd_release(old_fd);

	DEBUG("created fd with id %d\n", p->fd);

	return 0;
}

static int sc_hdlr_close(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	fs_filed_t *fd;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* handle close */
	// NOTE fs_fd_release must not be called, since
	// 		close has already deleted the decriptor
	return fd->node->ops->close(fd, this_p);
}

static int sc_hdlr_read(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	ssize_t r;
	char buf[p->payload_len];
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* call read callback if implemented */
	if(node->ops->read == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("offset %d, max-len %u\n", fd->fp, p->payload_len);

	mutex_lock(&node->mtx);

	while(1){
		r = node->ops->read(fd, buf, p->payload_len);

		if(r || errno || fs_fd_wait(fd, &node->datain_sig, &node->mtx) != 0)
			break;
	}

	p->payload_len = r;

	mutex_unlock(&node->mtx);

	DEBUG("read %d bytes, \"%s\"\n", r, strerror(errno));

	// avoid communicating end of resource to user space
	if(errno == E_END)
		reset_errno();

	/* update user space */
	if(errno == 0)
		(void)copy_to_user(p->payload, buf, p->payload_len, this_p);

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_write(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;
	char buf[p->payload_len];


	/* initials */
	this_p = sched_running()->parent;

	if(copy_from_user(buf, p->payload, p->payload_len, this_p) != 0)
		return -errno;

	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* handle write */
	if(node->ops->write == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("offset %d, len %u\n", fd->fp, p->payload_len);

	mutex_lock(&node->mtx);
	p->payload_len = node->ops->write(fd, buf, p->payload_len);
	mutex_unlock(&node->mtx);

	DEBUG("written %d bytes, \"%s\"\n", p->payload_len, strerror(errno));

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_ioctl(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	char payload[p->payload_len];
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;

	if(copy_from_user(payload, p->payload, p->payload_len, this_p) != 0)
		return -errno;

	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* call ioctl callback if implemented */
	if(node->ops->ioctl == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("cmd %d\n", p->cmd);

	mutex_lock(&node->mtx);
	(void)node->ops->ioctl(fd, p->cmd, payload, p->payload_len);
	mutex_unlock(&node->mtx);

	/* update user space */
	if(errno == 0)
		(void)copy_to_user(p->payload, payload, p->payload_len, this_p);

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_fcntl(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	char payload[p->payload_len];
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;

	if(copy_from_user(payload, p->payload, p->payload_len, this_p) != 0)
		return -errno;

	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* call fcntl callback if implemented */
	DEBUG("cmd %d\n", p->cmd);

	mutex_lock(&node->mtx);

	if(fcntl(fd, p->cmd, payload, this_p) == -E_NOIMP){
		if(node->ops->fcntl != 0x0){
			reset_errno();
			(void)node->ops->fcntl(fd, p->cmd, payload);
		}
	}

	mutex_unlock(&node->mtx);

	/* update user space */
	if(errno == 0)
		(void)copy_to_user(p->payload, payload, p->payload_len, this_p);

	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_mmap(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	fs_filed_t *fd;
	fs_node_t *node;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	node = fd->node;

	/* call ioctl callback if implemented */
	if(node->ops->mmap == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("len %zu\n", p->payload_len);

	mutex_lock(&node->mtx);

	if(p->payload == 0x0)	p->payload = node->ops->mmap(fd, p->payload_len);
	else					kmunmap(p->payload);

	mutex_unlock(&node->mtx);

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_rmnode(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	char path[p->payload_len];
	fs_node_t *root;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;

	if(copy_from_user(path, p->payload, p->payload_len, this_p) != 0)
		return -errno;

	DEBUG("%s\n", path);

	/* identify file system and call its rmnode callback */
	mutex_lock(&this_p->mtx);
	root = (path[0] == '/') ? (fs_root) : this_p->cwd;
	mutex_unlock(&this_p->mtx);

	if(root->ops->node_rm == 0x0)
		return_errno(E_NOIMP);

	return root->ops->node_rm(root, path);
}

static int sc_hdlr_chdir(void *param){
	sc_fs_t *p = (sc_fs_t*)param;
	char path[p->payload_len];
	fs_node_t *root;
	process_t *this_p;


	/* initials */
	this_p = sched_running()->parent;

	if(copy_from_user(path, p->payload, p->payload_len, this_p) != 0)
		return -errno;

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

static int fcntl(fs_filed_t *fd, int cmd, void *arg, process_t *this_p){
	f_mode_t *mode = (f_mode_t*)arg;


	/* handle file descriptor mode commands */
	switch(cmd){
	case F_MODE_GET:
		*mode = fd->mode;
		break;

	case F_MODE_SET:
		fd->mode = *mode;
		break;

	default:
		return_errno(E_NOIMP);
	}

	return 0;
}
