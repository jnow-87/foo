#include <arch/core.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/sched.h>
#include <kernel/syscall.h>
#include <kernel/rootfs.h>
#include <sys/file.h>
#include <sys/list.h>


/* local/static prototypes */
static int sc_hdlr_open(void *param);
static int sc_hdlr_close(void *param);
static int sc_hdlr_read(void *param);
static int sc_hdlr_write(void *param);
static int sc_hdlr_ioctl(void *param);
static int sc_hdlr_fcntl(void *param);
static int sc_hdlr_rmnode(void *param);
static int sc_hdlr_chdir(void *param);


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

	/* get file descriptor */
	fd = list_find_safe(this_p->fds, id, p->fd, &this_p->mtx);

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* call close callback if implemented */
	if(fd->node->ops->close == 0x0)
		return_errno(E_NOIMP);

	fs_lock();
	(void)fd->node->ops->close(fd, this_p);
	fs_unlock();

	return E_OK;
}

static int sc_hdlr_read(void *_p){
	char buf[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;

	/* get file descriptor */
	fd = list_find_safe(this_p->fds, id, p->fd, &this_p->mtx);

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* call read callback if implemented */
	if(fd->node->ops->read == 0x0)
		return_errno(E_NOIMP);

	mutex_lock(&fd->node->rd_mtx);
	p->data_len = fd->node->ops->read(fd, buf, p->data_len);
	mutex_unlock(&fd->node->rd_mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, buf, p->data_len, this_p);

	return -errno;
}

static int sc_hdlr_write(void *_p){
	char buf[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_fs_t*)_p;

	copy_from_user(buf, p->data, p->data_len, this_p);

	/* get file descriptor */
	fd = list_find_safe(this_p->fds, id, p->fd, &this_p->mtx);

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* call write callback if implemented */
	if(fd->node->ops->write == 0x0)
		return_errno(E_NOIMP);

	mutex_lock(&fd->node->wr_mtx);
	p->data_len = fd->node->ops->write(fd, buf, p->data_len);
	mutex_unlock(&fd->node->wr_mtx);

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

	copy_from_user(data, p->data, p->data_len, this_p);

	/* get file descriptor */
	fd = list_find_safe(this_p->fds, id, p->fd, &this_p->mtx);

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* call ioctl callback if implemented */
	if(fd->node->ops->ioctl == 0x0)
		return_errno(E_NOIMP);

	mutex_lock(&fd->node->wr_mtx);
	mutex_lock(&fd->node->rd_mtx);

	(void)fd->node->ops->ioctl(fd, p->cmd, data);

	mutex_unlock(&fd->node->rd_mtx);
	mutex_unlock(&fd->node->wr_mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);

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

	copy_from_user(data, p->data, p->data_len, this_p);

	/* get file descriptor */
	fd = list_find_safe(this_p->fds, id, p->fd, &this_p->mtx);

	if(fd == 0x0)
		return_errno(E_INVAL);

	/* call fcntl callback if implemented */
	if(fd->node->ops->fcntl == 0x0)
		return_errno(E_NOIMP);

	mutex_lock(&fd->node->wr_mtx);
	mutex_lock(&fd->node->rd_mtx);

	(void)fd->node->ops->fcntl(fd, p->cmd, data);

	mutex_unlock(&fd->node->rd_mtx);
	mutex_unlock(&fd->node->wr_mtx);

	/* update user space */
	if(errno == E_OK)
		copy_to_user(p->data, data, p->data_len, this_p);

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
