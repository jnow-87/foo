#include <arch/core.h>
#include <arch/mem.h>
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
	errno_t e;


	e = E_OK;
	e |= sc_register(SC_OPEN, sc_hdlr_open);
	e |= sc_register(SC_CLOSE, sc_hdlr_close);
	e |= sc_register(SC_READ, sc_hdlr_read);
	e |= sc_register(SC_WRITE, sc_hdlr_write);
	e |= sc_register(SC_IOCTL, sc_hdlr_ioctl);
	e |= sc_register(SC_FCNTL, sc_hdlr_fcntl);
	e |= sc_register(SC_RMNODE, sc_hdlr_rmnode);
	e |= sc_register(SC_CHDIR, sc_hdlr_chdir);

	return e;
}

kernel_init(0, init);

static int sc_hdlr_open(void *_p){
	char path[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_node_t *root;


	/* initials */
	p = (sc_fs_t*)_p;

	if(copy_from_user(path, p->data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;

	/* identify file system and call its open callback */
	root = (path[0] == '/') ? (fs_root) : current_thread[PIR]->parent->cwd;

	if(root->ops->open == 0x0)
		goto_errno(k_ok, E_NOIMP);

	p->fd = root->ops->open(root, path, p->mode);


k_ok:
	p->errno = errno;
	return E_OK;

err:
	p->errno = errno;
	return errno;
}

static int sc_hdlr_close(void *_p){
	sc_fs_t *p;
	fs_filed_t *fd;


	/* initials */
	p = (sc_fs_t*)_p;

	/* get file descriptor */
	fd = list_find(current_thread[PIR]->parent->fds, id, p->fd);

	if(fd == 0x0)
		goto_errno(k_ok, E_INVAL);

	/* call close callback if implemented */
	if(fd->node->ops->close == 0x0)
		goto_errno(k_ok, E_NOIMP);

	(void)fd->node->ops->close(fd);


k_ok:
	p->errno = errno;
	return E_OK;
}

static int sc_hdlr_read(void *_p){
	char buf[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;


	/* initials */
	p = (sc_fs_t*)_p;

	/* get file descriptor */
	fd = list_find(current_thread[PIR]->parent->fds, id, p->fd);

	if(fd == 0x0)
		goto_errno(k_ok, E_INVAL);

	/* call read callback if implemented */
	if(fd->node->ops->read == 0x0)
		goto_errno(err, E_NOIMP);

	p->data_len = fd->node->ops->read(fd, buf, p->data_len);

	if(errno != E_OK)
		goto k_ok;

	/* update user space */
	if(copy_to_user(p->data, buf, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;


k_ok:
	p->errno = errno;
	return E_OK;

err:
	p->errno = errno;
	return errno;
}

static int sc_hdlr_write(void *_p){
	char buf[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;


	/* initials */
	p = (sc_fs_t*)_p;

	if(copy_from_user(buf, p->data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;

	/* get file descriptor */
	fd = list_find(current_thread[PIR]->parent->fds, id, p->fd);

	if(fd == 0x0)
		goto_errno(k_ok, E_INVAL);

	/* call write callback if implemented */
	if(fd->node->ops->write == 0x0)
		goto_errno(k_ok, E_NOIMP);

	p->data_len = fd->node->ops->write(fd, buf, p->data_len);


k_ok:
	p->errno = errno;
	return E_OK;

err:
	p->errno = errno;
	return errno;
}

static int sc_hdlr_ioctl(void *_p){
	char data[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;


	/* initials */
	p = (sc_fs_t*)_p;

	if(copy_from_user(data, p->data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;

	/* get file descriptor */
	fd = list_find(current_thread[PIR]->parent->fds, id, p->fd);

	if(fd == 0x0)
		goto_errno(k_ok, E_INVAL);

	/* call ioctl callback if implemented */
	if(fd->node->ops->ioctl == 0x0)
		goto_errno(k_ok, E_NOIMP);

	(void)fd->node->ops->ioctl(fd, p->cmd, data);

	/* update user space */
	if(copy_to_user(p->data, data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;


k_ok:
	p->errno = errno;
	return E_OK;


err:
	p->errno = errno;
	return errno;
}

static int sc_hdlr_fcntl(void *_p){
	char data[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_filed_t *fd;


	/* initials */
	p = (sc_fs_t*)_p;

	if(copy_from_user(data, p->data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;

	/* get file descriptor */
	fd = list_find(current_thread[PIR]->parent->fds, id, p->fd);

	if(fd == 0x0)
		goto_errno(k_ok, E_INVAL);

	/* call fcntl callback if implemented */
	if(fd->node->ops->fcntl == 0x0)
		goto_errno(k_ok, E_NOIMP);

	(void)fd->node->ops->fcntl(fd, p->cmd, data);

	/* update user space */
	if(copy_to_user(p->data, data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;


k_ok:
	p->errno = errno;
	return E_OK;

err:
	p->errno = errno;
	return errno;
}

static int sc_hdlr_rmnode(void *_p){
	char path[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_node_t *root;


	/* initials */
	p = (sc_fs_t*)_p;

	if(copy_from_user(path, p->data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;

	/* identify file system and call its rmnode callback */
	root = (path[0] == '/') ? (fs_root) : current_thread[PIR]->parent->cwd;

	if(root->ops->rmnode == 0x0)
		goto_errno(k_ok, E_NOIMP);

	(void)root->ops->rmnode(root, path);


k_ok:
	p->errno = errno;
	return E_OK;

err:
	p->errno = errno;
	return errno;
}

static int sc_hdlr_chdir(void *_p){
	char path[((sc_fs_t*)(_p))->data_len];
	sc_fs_t *p;
	fs_node_t *root;


	/* initials */
	p = (sc_fs_t*)_p;

	if(copy_from_user(path, p->data, p->data_len, current_thread[PIR]->parent) != E_OK)
		goto err;

	/* identify file system and call its chdir callback */
	root = (path[0] == '/') ? (fs_root) : current_thread[PIR]->parent->cwd;

	if(root->ops->chdir == 0x0)
		goto_errno(k_ok, E_NOIMP);

	(void)root->ops->chdir(root, path);


k_ok:
	p->errno = errno;
	return E_OK;

err:
	p->errno = errno;
	return errno;
}
