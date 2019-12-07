/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/fs.h>
#include <kernel/rootfs.h>
#include <kernel/syscall.h>
#include <kernel/net.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/net.h>
#include <sys/string.h>
#include <sys/math.h>
#include <sys/list.h>
#include <sys/ringbuf.h>
#include <sys/errno.h>


/* local/static prototypes */
static int sc_hdlr_socket(void *_p);
static int sc_hdlr_recv(void *_p);
static int sc_hdlr_send(void *_p);

static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p);
static int close(fs_filed_t *fd, process_t *this_p);
static size_t read(fs_filed_t *fd, void *buf, size_t n);
static size_t write(fs_filed_t *fd, void *buf, size_t n);
static int ioctl(fs_filed_t *fd, int request, void *data);

static int connect(socket_t *sock, sock_addr_t *addr, size_t addr_len);
static int listen(socket_t *sock, int backlog);
static int accept(socket_t *sock, sock_addr_t *addr, size_t *addr_len);
static size_t recvfrom(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len);
static ssize_t sendto(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len);

static int netfs_node_create(socket_t *sock, process_t *this_p);


/* static variables */
static int netfs_id = 0;
static fs_node_t *netfs_root = 0x0;


/* global functions */
socket_t *netfs_get_socket(fs_filed_t *fd){
	socket_t *sock;


	sock = (socket_t*)fd->node->data;

	if(sock == 0x0)
		goto_errno(err, E_NOCONN);

	return sock;


err:
	return 0x0;
}

fs_node_t *netfs_get_fsnode(socket_t *sock){
	fs_node_t *node;


	fs_lock();
	node = list_find(netfs_root->childs, data, sock);
	fs_unlock();

	return node;
}


/* local functions */
static int init(void){
	int r;
	fs_ops_t ops;


	/* register syscalls */
	r = E_OK;

	r |= sc_register(SC_SOCKET, sc_hdlr_socket);
	r |= sc_register(SC_RECV, sc_hdlr_recv);
	r |= sc_register(SC_SEND, sc_hdlr_send);

	if(r != E_OK)
		goto err_0;

	/* register netfs */
	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;
	ops.node_rm = 0x0;
	ops.node_find = 0x0;

	netfs_id = fs_register(&ops);

	if(netfs_id < 0)
		goto err_1;

	/* allocate root node */
	netfs_root = rootfs_mkdir("/socket", fs_root->fs_id);

	if(netfs_root == 0x0)
		goto err_2;

	return E_OK;


err_2:
	fs_release(netfs_id);

err_1:
	sc_release(SC_SEND);
	sc_release(SC_RECV);
	sc_release(SC_SOCKET);

err_0:
	return -errno;
}

kernel_init(2, init);

static int sc_hdlr_socket(void *_p){
	sc_socket_t *p;
	process_t *this_p;
	socket_t *sock;
	sock_addr_t addr;


	p = (sc_socket_t*)_p;
	this_p = sched_running()->parent;

	/* allocate socket */
	copy_from_user(&addr, p->addr, sizeof(sock_addr_t), this_p);

	sock = socket_alloc(p->type, addr.domain, net_domain_cfg[addr.domain].addr_len);

	if(sock == 0x0)
		goto err_0;

	/* create netfs node */
	p->fd = netfs_node_create(sock, this_p);

	if(p->fd < 0)
		goto err_1;

	return E_OK;


err_1:
	socket_free(sock);

err_0:
	return -errno;
}

static int sc_hdlr_recv(void *_p){
	ssize_t r;
	char buf[((sc_socket_t*)(_p))->data_len];
	char _addr[((sc_socket_t*)(_p))->addr_len];
	sock_addr_t *addr;
	sc_socket_t *p;
	fs_filed_t *fd;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_socket_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	addr = 0x0;

	if(p->addr){
		addr = (sock_addr_t*)_addr;
		copy_from_user(addr, p->addr, p->addr_len, this_p);
	}

	/* call read callback if implemented */
	mutex_lock(&fd->node->rd_mtx);

	while(1){
		r = recvfrom(fd, buf, p->data_len, addr, (addr ? &p->addr_len : 0x0));

		if(r || errno || (fd->mode & O_NONBLOCK))
			break;

		ksignal_wait(&fd->node->rd_sig);
	}

	p->data_len = r;

	mutex_unlock(&fd->node->rd_mtx);

	DEBUG("recv %d bytes, \"%s\"\n", r, strerror(errno));

	// avoid communicating end of resource to user space
	if(errno == E_END)
		errno = E_OK;

	/* update user space */
	if(errno == E_OK){
		copy_to_user(p->data, buf, p->data_len, this_p);
		copy_to_user(p->addr, addr, p->addr_len, this_p);
	}

end:
	fs_fd_release(fd);

	return -errno;
}

static int sc_hdlr_send(void *_p){
	char buf[((sc_socket_t*)_p)->data_len];
	char _addr[((sc_socket_t*)_p)->addr_len];
	sc_socket_t *p;
	fs_filed_t *fd;
	process_t *this_p;
	sock_addr_t *addr;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_socket_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	addr = 0x0;

	if(p->addr){
		addr = (sock_addr_t*)_addr;
		copy_from_user(addr, p->addr, p->addr_len, this_p);
	}

	/* handle send */
	copy_from_user(buf, p->data, p->data_len, this_p);

	mutex_lock(&fd->node->wr_mtx);
	p->data_len = sendto(fd, buf, p->data_len, addr, p->addr_len);
	mutex_unlock(&fd->node->wr_mtx);

	DEBUG("sent %d bytes, \"%s\"\n", p->data_len, strerror(errno));

end:
	fs_fd_release(fd);

	return -errno;
}

static int open(fs_node_t *start, char const *path, f_mode_t mode, process_t *this_p){
	return_errno(E_NOSUP);
}

static int close(fs_filed_t *fd, process_t *this_p){
	socket_t *sock;


	sock = netfs_get_socket(fd);

	if(sock && sock->dev)
		sock->dev->ops.close(sock);

	fs_fd_free(fd, this_p);

	if(fd->node->ref_cnt == 0){
		fs_lock();
		fs_node_destroy(fd->node);
		fs_unlock();
	}

	socket_free(sock);

	return E_OK;
}

static size_t read(fs_filed_t *fd, void *buf, size_t n){
	return recvfrom(fd, buf, n, 0x0, 0x0);
}

static size_t write(fs_filed_t *fd, void *buf, size_t n){
	return sendto(fd, buf, n, 0x0, 0);
}

static int ioctl(fs_filed_t *fd, int request, void *_data){
	socket_t *sock;
	socket_ioctl_t *data;


	sock = netfs_get_socket(fd);

	if(sock == 0x0)
		return -errno;

	data = (socket_ioctl_t*)_data;

	switch(request){
	case IOCTL_CONNECT:
		return connect(sock, &data->addr, data->addr_len);

	case IOCTL_BIND:
		return netdev_bind(sock, &data->addr, data->addr_len);

	case IOCTL_LISTEN:
		return listen(sock, *((int*)_data));

	case IOCTL_ACCEPT:
		data->fd = accept(sock, &data->addr, &data->addr_len);
		return data->fd > 0 ? E_OK : -errno;

	default:
		return_errno(E_NOSUP);
	}
}

static int connect(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	netdev_t *dev;


	if(sock->dev != 0x0)
		return_errno(E_CONN);

	if(netdev_bind(sock, addr, addr_len) != 0)
		return -errno;

	dev = sock->dev;

	if(dev->ops.connect == 0x0)
		return_errno(E_NOSUP);

	if(dev->ops.connect(sock) != 0)
		return -errno;

	return 0;
}

static int listen(socket_t *sock, int backlog){
	netdev_t *dev;


	dev = sock->dev;

	if(sock->type != SOCK_STREAM)
		return_errno(E_INVAL);

	if(dev == 0x0)
		return_errno(E_NOCONN);

	if(dev->ops.listen == 0x0)
		return_errno(E_NOSUP);

	return dev->ops.listen(sock, backlog);
}

static int accept(socket_t *sock, sock_addr_t *addr, size_t *addr_len){
	int fd_id;
	socket_t *cl;
	netdev_t *dev;


	dev = sock->dev;

	if(sock->type != SOCK_STREAM)
		return_errno(E_INVAL);

	if(dev == 0x0)
		return_errno(E_NOCONN);

	/* get client socket */
	cl = list_first_safe(sock->clients, sock->mtx);

	if(cl == 0x0)
		return_errno(E_END);

	if(addr_len && addr){
		memcpy(addr, &cl->addr, *addr_len);
		*addr_len = net_domain_cfg[cl->addr.domain].addr_len;
	}

	cl->dev = dev;

	/* create netfs node */
	fd_id = netfs_node_create(cl, sched_running()->parent);

	if(fd_id < 0)
		return -errno;

	list_rm_safe(sock->clients, cl, sock->mtx);

	return fd_id;
}

static size_t recvfrom(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len){
	size_t n;
	socket_t *sock;
	datagram_t *dgram;


	sock = (socket_t*)fd->node->data;

	if(sock == 0x0)
		goto_errno(err, E_NOCONN);

	if(sock->type == SOCK_DGRAM){
		dgram = list_first(sock->dgrams);

		if(dgram == 0x0)
			return 0;

		if(addr_len && addr){
			memcpy(addr, &dgram->addr, *addr_len);
			*addr_len = net_domain_cfg[sock->addr.domain].addr_len;
		}

		n = MIN(data_len, dgram->len - dgram->idx);
		memcpy(data, dgram->data + dgram->idx, n);
		dgram->idx += n;

		if(dgram->idx == dgram->len){
			list_rm(sock->dgrams, dgram);
			kfree(dgram);
		}

		return n;
	}
	else
		return ringbuf_read(&sock->stream, data, data_len);


err:
	return 0;
}

static ssize_t sendto(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len){
	socket_t *sock;
	netdev_t *dev;


	sock = (socket_t*)fd->node->data;

	if(sock == 0x0)
		goto_errno(err, E_NOCONN);

	dev = sock->dev;

	if(sock->type == SOCK_STREAM){
		if(dev == 0x0)
			goto_errno(err, E_NOCONN);

		if(addr != 0x0)
			goto_errno(err, E_CONN);
	}
	else{
		if(addr){
			if(netdev_bind(sock, addr, addr_len) != 0)
				goto err;
		}
		else if(dev == 0x0)
			goto_errno(err, E_NOCONN);

		dev = sock->dev;
	}

	if(dev->ops.send == 0x0)
		goto_errno(err, E_NOSUP);

	return dev->ops.send(sock, data, data_len);


err:
	return 0;
}

static int netfs_node_create(socket_t *sock, process_t *this_p){
	static uint8_t sock_id = 0;
	char name[4];
	fs_node_t *node;
	fs_filed_t *fd;


	/* create netfs node */
	sock_id++;

	if(sock_id == 0)
		return_errno(E_LIMIT);

	itoa(sock_id, 10, name, 4);

	fs_lock();
	node = fs_node_create(netfs_root, name, strlen(name), FT_REG, sock, netfs_id);
	fs_unlock();

	if(node == 0x0)
		goto err_0;

	/* create file descriptor */
	fd = fs_fd_alloc(node, this_p, O_RDWR, 0x0);

	if(fd == 0x0)
		goto err_1;

	sock->mtx = &node->rd_mtx;

	 return fd->id;


err_1:
	fs_lock();
	fs_node_destroy(node);
	fs_unlock();

err_0:
	return -errno;
}
