#include <arch/core.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/sched.h>
#include <kernel/syscall.h>
#include <kernel/rootfs.h>
#include <kernel/task.h>
#include <kernel/signal.h>
#include <kernel/kprintf.h>
#include <sys/fcntl.h>
#include <sys/list.h>


/* macros */
// read/write specific debug macros
#ifdef CONFIG_KERNEL_FS_DEBUG_RDWR
#define DEBUGRDWR(fmt, ...)		cprintf(KMSG_DEBUG, "[DBG] %25.25s:%-20.20s    "fmt, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define DEBUGRDWR(fmt, ...)		{}
#endif


/* types */
typedef void (*task_hdlr_t)(sc_fs_t *p, fs_filed_t *fd, process_t *this_p);

typedef struct{
	int fileno;
	process_t *this_p;
	sc_fs_t sc_param;

	task_hdlr_t op;
} task_param_t;


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
static void write(sc_fs_t *p, fs_filed_t *fd, process_t *this_p);
static int fcntl(fs_filed_t *fd, int cmd, void *data, process_t *this_p);

// kernel task handling
static void task_create(task_hdlr_t op, sc_fs_t *sc_p, fs_filed_t *fd, process_t *this_p);
static void task_hdlr(void *_p);


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

	DEBUG("sc open: path \"%s\", mode %#x\n", path, p->mode);

	fs_lock();
	p->fd = root->ops->open(root, path, p->mode, this_p);
	fs_unlock();

	DEBUG("created fd with id %d, errno %#x\n", p->fd, errno);

	return E_OK;
}

static int sc_hdlr_dup(void *_p){
	int old_id;
	sc_fs_t *p;
	fs_filed_t *old_fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initiali */
	p = (sc_fs_t*)_p;
	old_id = (int)p->data;
	old_fd = fs_fd_acquire(old_id, this_p);

	DEBUG("sc dup: oldfd %d%s, newfd %d\n", old_id, (old_fd == 0x0 ? " (invalid)" : ""), p->fd);

	// exit if oldfd does not exist or old and new fd are the same
	if(old_fd == 0x0 || old_fd->id == p->fd)
		return_errno(E_INVAL);

	/* close the desired fd if one is given */
	if(p->fd >= 0)
		sc_hdlr_close(_p);

	// E_INVAL is expected in case the desired fd
	// was not open before
	if(errno & (~E_INVAL))
		return -errno;

	errno = E_OK;

	/* duplicate old fd */
	fs_lock();
	p->fd = fs_fd_dup(old_fd, p->fd, this_p);
	fs_unlock();

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

	DEBUG("sc close: fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* handle close */
	fs_fd_release(fd);

	// wait for outstanding tasks to finish
	ktask_queue_flush(fd->tasks);

	fd = fs_fd_acquire(p->fd, this_p);

	fs_lock();
	(void)fd->node->ops->close(fd, this_p);
	fs_unlock();

	// NOTE fs_fd_release must not be called, since
	// 		close has already deleted the decriptor

	return E_OK;
}

static int sc_hdlr_read(void *_p){
	int r;
	char buf[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUGRDWR("sc read: fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* call read callback if implemented */
	if(fd->node->ops->read == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUGRDWR("offset %d, max-len %u\n", fd->fp, p->data_len);

	mutex_lock(&fd->node->rd_mtx);

	while(1){
		r = fd->node->ops->read(fd, buf, p->data_len);

		if(r || errno || (fd->mode & O_NONBLOCK))
			break;

		ksignal_wait(&fd->node->rd_sig);
	}

	p->data_len = r;

	mutex_unlock(&fd->node->rd_mtx);

	DEBUGRDWR("read %d bytes, errno %#x\n", r, errno);

	// avoid communicating enf of resource to user space
	errno &= ~E_END;

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
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUGRDWR("sc write: fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* handle write */
	if(fd->node->ops->write == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUGRDWR("offset %d, len %u\n", fd->fp, p->data_len);

	if(fd->mode & O_NONBLOCK)	task_create(write, p, fd, this_p);
	else						write(p, fd, this_p);

	DEBUGRDWR("written %d bytes, errno %#x\n", p->data_len, errno);

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_ioctl(void *_p){
	char data[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd =  fs_fd_acquire(p->fd, this_p);

	DEBUG("sc ioctl: fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	copy_from_user(data, p->data, p->data_len, this_p);

	/* call ioctl callback if implemented */
	if(fd->node->ops->ioctl == 0x0)
		goto_errno(end, E_NOIMP);

	DEBUG("cmd %d\n", p->cmd);

	mutex_lock(&fd->node->wr_mtx);
	mutex_lock(&fd->node->rd_mtx);

	(void)fd->node->ops->ioctl(fd, p->cmd, data);

	mutex_unlock(&fd->node->rd_mtx);
	mutex_unlock(&fd->node->wr_mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);

	DEBUG("errno %#x\n", errno);

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_fcntl(void *_p){
	char data[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("sc fcntl: fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	copy_from_user(data, p->data, p->data_len, this_p);

	/* call fcntl callback if implemented */
	DEBUG("cmd %d\n", p->cmd);

	mutex_lock(&fd->node->wr_mtx);
	mutex_lock(&fd->node->rd_mtx);

	if(fcntl(fd, p->cmd, data, this_p) != E_OK){
		if(fd->node->ops->fcntl != 0x0)		(void)fd->node->ops->fcntl(fd, p->cmd, data);
		else								errno |= E_NOIMP;
	}

	mutex_unlock(&fd->node->rd_mtx);
	mutex_unlock(&fd->node->wr_mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);

	fs_fd_release(fd);

	DEBUG("errno %#x\n", errno);

	return -errno;
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
	DEBUG("sc rmnode: %s\n", path);

	/* identify file system and call its rmnode callback */
	mutex_lock(&this_p->mtx);
	root = (path[0] == '/') ? (fs_root) : this_p->cwd;
	mutex_unlock(&this_p->mtx);

	if(root->ops->node_rm == 0x0)
		return_errno(E_NOIMP);

	fs_lock();
	(void)root->ops->node_rm(root, path);
	fs_unlock();

	DEBUG("errno %#x\n", errno);

	return -errno;
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

	// update current working directory
	this_p->cwd->ref_cnt--;
	this_p->cwd = root;
	root->ref_cnt++;


end:
	mutex_unlock(&this_p->mtx);
	fs_unlock();

	DEBUG("new cwd \"%s\", errno %#x\n", this_p->cwd, errno);

	return -errno;
}

static void write(sc_fs_t *p, fs_filed_t *fd, process_t *this_p){
	char buf[p->data_len];


	/* initials */
	copy_from_user(buf, p->data, p->data_len, this_p);

	/* call write callback */
	mutex_lock(&fd->node->wr_mtx);

	p->data_len = fd->node->ops->write(fd, buf, p->data_len);
	ksignal_send(&fd->node->rd_sig);

	mutex_unlock(&fd->node->wr_mtx);
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
		fd->mode = *mode;
		break;

	case F_SYNC:
		mutex_unlock(&fd->node->rd_mtx);
		mutex_unlock(&fd->node->wr_mtx);
		fs_fd_release(fd);

		ktask_queue_flush(fd->tasks);

		fs_fd_acquire(fd->id, this_p);
		mutex_lock(&fd->node->wr_mtx);
		mutex_lock(&fd->node->rd_mtx);
		break;

	default:
		return -E_NOIMP;
	}

	return E_OK;
}

static void task_create(task_hdlr_t op, sc_fs_t *sc_p, fs_filed_t *fd, process_t *this_p){
	task_param_t tp;


	/* init task data */
	tp.fileno = fd->id;
	tp.this_p = this_p;
	tp.op = op;
	memcpy(&tp.sc_param, sc_p, sizeof(sc_fs_t));

	/* create task */
	ktask_create(task_hdlr, &tp, sizeof(task_param_t), fd->tasks, false);
}

static void task_hdlr(void *_p){
	fs_filed_t *fd;
	task_param_t *p;


	/* initials */
	p = (task_param_t*)_p;
	fd = fs_fd_acquire(p->fileno, p->this_p);

	if(fd == 0x0)
		return;

	/* handle operation */
	p->op(&p->sc_param, fd, p->this_p);

	fs_fd_release(fd);
}
