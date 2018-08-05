#include <arch/core.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/sched.h>
#include <kernel/syscall.h>
#include <kernel/rootfs.h>
#include <kernel/task.h>
#include <kernel/signal.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/list.h>


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
static int sc_hdlr_close(void *param);
static int sc_hdlr_read(void *param);
static int sc_hdlr_write(void *param);
static int sc_hdlr_ioctl(void *param);
static int sc_hdlr_fcntl(void *param);
static int sc_hdlr_rmnode(void *param);
static int sc_hdlr_chdir(void *param);

// actual operations
static void close(sc_fs_t *p, fs_filed_t *fd, process_t *this_p);
static void write(sc_fs_t *p, fs_filed_t *fd, process_t *this_p);
static int fcntl(fs_filed_t *fd, int cmd, void *data);

// kernel task handling
static void task_create(task_hdlr_t op, sc_fs_t *sc_p, fs_filed_t *fd, process_t *this_p);
static void task_hdlr(void *_p);


/* local functions */
static int init(void){
	int r;


	r = E_OK;

	r |= sc_register(SC_OPEN, sc_hdlr_open);
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

	fs_lock();
	p->fd = root->ops->open(root, path, p->mode, this_p);
	fs_unlock();

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

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* handle close */
	if(fd->mode & O_NONBLOCK){
		task_create(close, p, fd, this_p);
		fs_fd_release(fd);
	}
	else{
		close(p, fd, this_p);
		// NOTE fs_fd_release must not be called here,
		//		since close deletes the decriptor
	}

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

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* call read callback if implemented */
	if(fd->node->ops->read == 0x0)
		goto_errno(end, E_NOIMP);

	mutex_lock(&fd->node->rd_mtx);

	while(1){
		r = fd->node->ops->read(fd, buf, p->data_len);

		if(r || errno || (fd->mode & O_NONBLOCK))
			break;

		ksignal_wait(&fd->node->rd_sig);
	}

	p->data_len = r;

	mutex_unlock(&fd->node->rd_mtx);

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

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* handle write */
	if(fd->node->ops->write == 0x0)
		goto_errno(end, E_NOIMP);

	if(fd->mode & O_NONBLOCK)	task_create(write, p, fd, this_p);
	else						write(p, fd, this_p);


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

	if(fd == 0x0)
		return_errno(E_INVAL);

	copy_from_user(data, p->data, p->data_len, this_p);

	/* call ioctl callback if implemented */
	if(fd->node->ops->ioctl == 0x0)
		goto_errno(end, E_NOIMP);

	mutex_lock(&fd->node->wr_mtx);
	mutex_lock(&fd->node->rd_mtx);

	(void)fd->node->ops->ioctl(fd, p->cmd, data);

	mutex_unlock(&fd->node->rd_mtx);
	mutex_unlock(&fd->node->wr_mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);


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

	if(fd == 0x0)
		return_errno(E_INVAL);

	copy_from_user(data, p->data, p->data_len, this_p);

	/* call fcntl callback if implemented */
	mutex_lock(&fd->node->wr_mtx);
	mutex_lock(&fd->node->rd_mtx);

	if(fcntl(fd, p->cmd, data) != E_OK){
		if(fd->node->ops->fcntl != 0x0)		(void)fd->node->ops->fcntl(fd, p->cmd, data);
		else								errno |= E_NOIMP;
	}

	mutex_unlock(&fd->node->rd_mtx);
	mutex_unlock(&fd->node->wr_mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);

	fs_fd_release(fd);

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

	/* identify file system and call its rmnode callback */
	mutex_lock(&this_p->mtx);
	root = (path[0] == '/') ? (fs_root) : this_p->cwd;
	mutex_unlock(&this_p->mtx);

	if(root->ops->node_rm == 0x0)
		return_errno(E_NOIMP);

	fs_lock();
	(void)root->ops->node_rm(root, path);
	fs_unlock();

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

	return -errno;
}

static void close(sc_fs_t *p, fs_filed_t *fd, process_t *this_p){
	/* wait for outstanding operations to finish */
	while(fd->outstanding){
		fs_fd_release(fd);
		sched_yield();
		fs_fd_acquire(p->fd, this_p);
	}

	/* handle close */
	fs_lock();
	(void)fd->node->ops->close(fd, this_p);
	fs_unlock();
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

static int fcntl(fs_filed_t *fd, int cmd, void *data){
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

	/* signal outstanding operations */
	fd->outstanding++;

	/* create task */
	ktask_create(task_hdlr, &tp, sizeof(task_param_t));
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
	// pre-decrement in order for close() not to wait on itselft
	fd->outstanding--;
	p->op(&p->sc_param, fd, p->this_p);

	// descriptor must not be released for close, hence it has
	// already been deleted through the call
	if(p->op != close)
		fs_fd_release(fd);
}
