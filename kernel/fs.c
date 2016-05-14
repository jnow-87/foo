// TODO check implementation
#include <arch/core.h>
#include <arch/umem.h>
#include <kernel/fs.h>
#include <kernel/kmem.h>
#include <kernel/syscall.h>
#include <kernel/process.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <kernel/init.h>
#include <driver/rootfs.h>
#include <sys/string.h>
#include <sys/error.h>


/* static variables */
static fs_t fs_list = LIST_INITIALISER(&fs_list);
static int fs_id = 0;


/* local/static prototypes */
static error_t fs_init(void);
static error_t sc_hdlr_open(void*);
static error_t sc_hdlr_close(void*);
static error_t sc_hdlr_read(void*);
static error_t sc_hdlr_write(void*);
static error_t sc_hdlr_ioctl(void*);
static error_t sc_hdlr_fcntl(void*);
static error_t sc_hdlr_rmnode(void*);
static error_t sc_hdlr_chdir(void*);


/* global functions */
/**
 * \brief	register a new filesystem
 *
 * \param	ops	pointer to struct with this filesystems operations
 *
 * \return	the filesystems type (>0)
 * 			-1	invalid ops
 * 			-2	max. number of filesystem ids used
 * 			-3	out of kernel memory
 */
int fs_register(fs_ops_t *ops){
	fs_t *new;

	if(ops == 0)
		return E_INVAL;

	if(fs_id >= 0x7fffffff)
		return E_INVAL;

	new = kmalloc(sizeof(fs_t));
	if(new == 0)
		return E_NOMEM;

	new->fs_type = fs_id++;
	new->ops = kmalloc(sizeof(fs_ops_t));

	if(new->ops == 0)
		return E_NOMEM;

	memcpy(new->ops, ops, sizeof(fs_ops_t));
	list_add_tail(&fs_list, new);

	return new->fs_type;
}

/**
 * \brief	unregister a filesystem
 *
 * \param	fs_type		type returned from fs_register()
 *
 * \return	0
 */
error_t fs_unregister(int fs_type){
	fs_t *fs;


	fs = list_find(&fs_list, fs_type, fs_type);

	if(fs != 0){
		list_rm(&fs_list, fs);
		kfree(fs);
	}

	return E_OK;
}

/**
 * \brief	get ops by filesystem id
 *
 * \param	fs_type		filesystems id
 * \return	pointer to callback function struct
 * 			0 if no entry found
 */
fs_ops_t *fs_get_ops(int fs_type){
	fs_t *fs;
	
	fs = list_find(&fs_list, fs_type, fs_type);
	if(fs != 0)
		return fs->ops;
	return 0;
}

/**
 * \brief	set a processes CWD
 *
 * \param	node	the target CWD (if 0 it is set to root)
 *
 * \return	pointer to cwd
 */
fs_node_t *fs_get_cwd(fs_node_t *node){
	fs_node_t *cwd;

	cwd = node;
	if(cwd == 0)
		cwd = &root;
	return cwd;
}

/**
 * \brief	create new filed for the given node
 * 			and add it to the currents process fd list
 *
 * \param	node	fs_node for whom to create fd
 *
 * \return	0		error
 * 			else	pointer to filed
 */
fs_filed_t *fs_mkfd(fs_node_t *node, void *data){
	int fdi;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = current_process[PIR];

	// try to get new file descriptor
	fdi = 1;
	list_for_each(&this_p->fds, fd){
		if(fd->fd >= fdi)
			fdi = fd->fd + 1;
	}

	if(fdi < 1){
		WARN("out of file handles\n");
		goto err;
	}

	fd = kmalloc(sizeof(fs_filed_t));
	if(fd == 0){
		WARN("out of kernel memory\n");
		goto err;
	}

	node->ref_cnt++;
	fd->node = node;
	fd->data = data;
	fd->fd = fdi;
	fd->fp = 0;

	list_add_tail(&this_p->fds, fd);

	return fd;

err:
	return 0;
}

/**
 * \brief	free filed and remove from processes
 * 			fd list, fd.data has to be freed by caller
 *
 * \param	fd	filed to free
 */
void fs_rmfd(fs_filed_t *fd){
	process_t *this_p;


	this_p = current_process[PIR];
	
	fd->node->ref_cnt--;

	// remove fd from processes fd list
	list_rm(&this_p->fds, fd);
	kfree(fd);
}

/**
 * \brief	free open fileds - to be used when destroying a process
 * 			to free fd.data the actual close() method of
 * 			fd.node is called
 *
 * \param	fds		pointer to file descriptor list
 */
void fs_cleanup_fds(fs_filed_t *fds, unsigned int pid){
	fs_filed_t *fd;
	fs_ops_t *ops;


	// set current_process, because fs_rmfd (maybe called by ops->close())
	// requires a valid pid and fs_cleanup_fds() shall be used by
	// process_destroy(), but when it is active current_process is set to zero
	current_process[PIR] = list_find(&process_table, pid, pid);

	while(1){
		fd = list_first(fds);
		if(fd == 0)
			break;

		ops = fs_get_ops(fd->node->fs_type);
		if(ops != 0){
			if(ops->close != 0)
				ops->close(fd);
		}
	}

	current_process[PIR] = 0;
}


/* local functions */
static error_t fs_init(void){
	sc_hdlr_register(SC_OPEN, sc_hdlr_open);
	sc_hdlr_register(SC_CLOSE, sc_hdlr_close);
	sc_hdlr_register(SC_READ, sc_hdlr_read);
	sc_hdlr_register(SC_WRITE, sc_hdlr_write);
	sc_hdlr_register(SC_IOCTL, sc_hdlr_ioctl);
	sc_hdlr_register(SC_FCNTL, sc_hdlr_fcntl);
	sc_hdlr_register(SC_RMNODE, sc_hdlr_rmnode);
	sc_hdlr_register(SC_CHDIR, sc_hdlr_chdir);

	return E_OK;
}

kernel_init(1, fs_init);


static error_t sc_hdlr_open(void *_p){
	char *path;
	error_t e;
	sc_param_open_t *p;
	fs_node_t *start;
	fs_ops_t *ops;
	process_t *this_p;


	/* initials */
	p = (sc_param_open_t*)_p;
	this_p = current_process[PIR];

	path = kmalloc(p->path_len + 1);
	if(path == 0){
		WARN("out of kernel memory\n");
		e = E_NOMEM;
		goto err_0;
	}

	arch_copy_from_user(path, p->path, p->path_len + 1, this_p);

	DEBUG("fd %d, pid %d\n", p->fd, this_p->pid);

	/* identify filesystem and call its open callback */
	start = (path[0] == '/') ? &root : this_p->cwd;

	ops = fs_get_ops(start->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", start->fs_type);
		goto ok;
	}

	if(ops->open == 0){
		WARN("open not implemented for this file\n");
		goto ok;
	}

	p->fd = ops->open(start, path, p->mode);

ok:
	kfree(path);

	return E_OK;

err_0:
	p->fd = 0;

	return E_OK;
}

static error_t sc_hdlr_close(void *_p){
	error_t e;
	sc_param_close_t *p;
	process_t *this_p;
	fs_filed_t *fd;
	fs_ops_t *ops;


	p = (sc_param_close_t*)_p;
	this_p = current_process[PIR];

	DEBUG("fd %d, pid %d\n", p->fd, this_p->pid);

	fd = list_find(&this_p->fds, fd, p->fd);

	ops = fs_get_ops(fd->node->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", fd->node->fs_type);
		e = E_INVAL;
		goto err;
	}

	if(ops->close == 0){
		WARN("close not implemented for this file\n");
		e = E_INVAL;
		goto err;
	}

	p->ret = ops->close(fd);

	return E_OK;

err:
	p->ret = e;

	return E_OK;
}

static error_t sc_hdlr_read(void *_p){
	char *buf;
	sc_param_read_t *p;
	process_t *this_p;
	fs_filed_t *fd;
	fs_ops_t *ops;


	p = (sc_param_read_t*)_p;
	this_p = current_process[PIR];

	DEBUG("fd %d, pid %d, n %d\n", p->fd, this_p->pid, p->n);

	fd = list_find(&this_p->fds, fd, p->fd);

	ops = fs_get_ops(fd->node->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", fd->node->fs_type);
		p->ret = E_INVAL;
		goto end;
	}

	if(ops->read == 0){
		WARN("read not implemented for this file\n");
		p->ret = E_INVAL;
		goto end;
	}

	buf = kmalloc(p->n);
	if(buf == 0){
		WARN("out of kernel memory\n");
		p->ret = E_NOMEM;
		goto end;
	}

	p->ret = ops->read(fd, buf, p->n);

	if(p->ret > 0)
		arch_copy_to_user(p->buf, buf, p->ret, this_p);

	kfree(buf);

end:
	return E_OK;
}

static error_t sc_hdlr_write(void *_p){
	char *buf;
	sc_param_write_t *p;
	process_t *this_p;
	fs_filed_t *fd;
	fs_ops_t *ops;


	p = (sc_param_write_t*)_p;
	this_p = current_process[PIR];

	DEBUG("fd %d, pid %d, n %d\n", p->fd, this_p->pid, p->n);

	fd = list_find(&this_p->fds, fd, p->fd);

	ops = fs_get_ops(fd->node->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", fd->node->fs_type);
		p->ret = E_INVAL;
		goto end;
	}

	if(ops->write == 0){
		WARN("write not implemented for this file\n");
		p->ret = E_INVAL;
		goto end;
	}

	buf = kmalloc(p->n);
	if(buf == 0){
		WARN("out of kernel memory\n");
		p->ret = E_NOMEM;
		goto end;
	}

	arch_copy_from_user(buf, p->buf, p->n, this_p);
	p->ret = ops->write(fd, buf, p->n);
	kfree(buf);

end:
	return E_OK;
}

static error_t sc_hdlr_ioctl(void *_p){
	sc_param_ioctl_t *p;
	process_t *this_p;
	fs_filed_t *fd;
	fs_ops_t *ops;


	p = (sc_param_ioctl_t*)_p;
	this_p = current_process[PIR];

	DEBUG("fd %d, pid %d, req %d\n", p->fd, this_p->pid, p->request);

	fd = list_find(&this_p->fds, fd, p->fd);

	ops = fs_get_ops(fd->node->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", fd->node->fs_type);
		p->ret = E_INVAL;
		goto end;
	}

	if(ops->ioctl == 0){
		WARN("ioctl not implemented for this file\n");
		p->ret = E_INVAL;
		goto end;
	}

	p->ret = ops->ioctl(fd, p->request, p->param);

end:
	return E_OK;
}

static error_t sc_hdlr_fcntl(void *_p){
	sc_param_fcntl_t *p;
	process_t *this_p;
	fs_filed_t *fd;
	fs_ops_t *ops;


	p = (sc_param_fcntl_t*)_p;
	this_p = current_process[PIR];

	DEBUG("fd %d, pid %d, cmd %d\n", p->fd, this_p->pid, p->cmd);

	fd = list_find(&this_p->fds, fd, p->fd);

	ops = fs_get_ops(fd->node->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", fd->node->fs_type);
		p->ret = E_INVAL;
		goto end;
	}

	if(ops->fcntl == 0){
		WARN("fcntl not implemented for this file\n");
		p->ret = E_INVAL;
		goto end;
	}

	p->ret = ops->fcntl(fd, p->cmd, p->param);

end:
	return E_OK;
}

static error_t sc_hdlr_rmnode(void *_p){
	char *path;
	sc_param_rmnode_t *p;
	process_t *this_p;
	fs_ops_t *ops;
	fs_node_t *start;


	/* initials */
	p = (sc_param_rmnode_t*)_p;
	this_p = current_process[PIR];

	path = kmalloc(p->path_len + 1);
	if(path == 0){
		WARN("out of kernel memory\n");
		p->ret = E_NOMEM;
		goto err_0;
	}

	arch_copy_from_user(path, p->path, p->path_len + 1, this_p);

	DEBUG("\"%s\", pid %d\n", path, this_p->pid);

	/* identify filesystem and call its open callback */
	start = (path[0] == '/') ? &root : this_p->cwd;

	ops = fs_get_ops(start->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", start->fs_type);
		p->ret = E_INVAL;
		goto err_1;
	}

	if(ops->rmnode == 0){
		WARN("rmnode not implemented for this file\n");
		p->ret = E_INVAL;
		goto err_1;
	}

	p->ret = ops->rmnode(start, path);
	kfree(path);

	return E_OK;

err_1:
	kfree(path);

err_0:
	return E_OK;
}

static error_t sc_hdlr_chdir(void *_p){
	char *path;
	sc_param_chdir_t *p;
	process_t *this_p;
	fs_node_t *start;
	fs_ops_t *ops;


	p = (sc_param_chdir_t*)_p;
	this_p = current_process[PIR];

	path = kmalloc(p->path_len + 1);
	if(path == 0){
		WARN("out of kernel memory\n");
		p->ret = E_NOMEM;
		goto err_0;
	}

	arch_copy_from_user(path, p->path, p->path_len + 1, this_p);

	DEBUG("\"%s\", pid %d\n", path, this_p->pid);

	start = (path[0] == '/') ? &root : this_p->cwd;

	ops = fs_get_ops(start->fs_type);
	if(ops == 0){
		WARN("no operations found for fs_type %d\n", start->fs_type);
		p->ret = E_INVAL;
		goto err_1;
	}

	if(ops->chdir == 0){
		WARN("chdir not implemented for this file\n");
		p->ret = E_INVAL;
		goto err_1;
	}

	p->ret = ops->chdir(start, path);
	kfree(path);

	return E_OK;

err_1:
	kfree(path);

err_0:
	return E_OK;
}
