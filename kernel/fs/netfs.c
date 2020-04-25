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
#include <kernel/critsec.h>
#include <kernel/ksignal.h>
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
static int bind(socket_t *sock, sock_addr_t *addr, size_t addr_len);
static int listen(socket_t *sock, int backlog);
static int accept(socket_t *sock, sock_addr_t *addr, size_t *addr_len);
static size_t recvfrom(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len);
static ssize_t sendto(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len);
static int free_client(socket_t *sock);

static int link_to_fs(socket_t *sock, process_t *this_p);


/* static variables */
static int netfs_id = 0;
static fs_node_t *netfs_root = 0x0;
static mutex_t netfs_mtx = MUTEX_INITIALISER();


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
	p->fd = link_to_fs(sock, this_p);

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
	fs_node_t *node;
	process_t *this_p;


	this_p = sched_running()->parent;

	/* initials */
	p = (sc_socket_t*)_p;
	fd = fs_fd_acquire(p->fd, this_p);

	DEBUG("fd %d%s\n", p->fd, (fd == 0x0 ? " (invalid)" : ""));

	if(fd == 0x0)
		return_errno(E_INVAL);

	addr = 0x0;
	node = fd->node;

	if(p->addr){
		addr = (sock_addr_t*)_addr;
		copy_from_user(addr, p->addr, p->addr_len, this_p);
	}

	/* call read callback if implemented */
	mutex_lock(&node->mtx);

	while(1){
		r = recvfrom(fd, buf, p->data_len, addr, (addr ? &p->addr_len : 0x0));

		if(r || errno || (fd->mode & O_NONBLOCK))
			break;

		ksignal_wait_mtx(&node->datain_sig, &node->mtx);
	}

	p->data_len = r;

	mutex_unlock(&node->mtx);

	DEBUG("recv %d bytes, \"%s\"\n", r, strerror(errno));

	// avoid communicating end of resource to user space
	if(errno == E_END)
		errno = E_OK;

	/* update user space */
	if(errno == E_OK){
		copy_to_user(p->data, buf, p->data_len, this_p);

		if(addr)
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

	mutex_lock(&fd->node->mtx);
	p->data_len = sendto(fd, buf, p->data_len, addr, p->addr_len);
	mutex_unlock(&fd->node->mtx);

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


	fs_lock();

	sock = (socket_t*)fd->node->data;

	if(sock->dev)
		sock->dev->ops.close(sock);

	fs_fd_free(fd, this_p);

	if(fs_node_destroy(fd->node) != 0)
		goto end;

	while(sock->type == SOCK_STREAM){
		if(free_client(socket_get_client_socket(sock)) != 0)
			break;
	}

	socket_free(sock);

end:
	fs_unlock();

	return E_OK;
}

static size_t read(fs_filed_t *fd, void *buf, size_t n){
	return recvfrom(fd, buf, n, 0x0, 0x0);
}

static size_t write(fs_filed_t *fd, void *buf, size_t n){
	return sendto(fd, buf, n, 0x0, 0);
}

static int ioctl(fs_filed_t *fd, int request, void *_data){
	int r;
	socket_t *sock;
	socket_ioctl_t *data;


	data = (socket_ioctl_t*)_data;
	sock = (socket_t*)fd->node->data;

	mutex_lock(&sock->mtx);

	switch(request){
	case IOCTL_CONNECT:
		r = connect(sock, &data->addr, data->addr_len);
		break;

	case IOCTL_BIND:
		r = bind(sock, &data->addr, data->addr_len);
		break;

	case IOCTL_LISTEN:
		r = listen(sock, *((int*)_data));
		break;

	case IOCTL_ACCEPT:
		while(1){
			data->fd = accept(sock, &data->addr, &data->addr_len);

			if(data->fd >= 0 || errno || (fd->mode & O_NONBLOCK))
				break;

			ksignal_wait_mtx(&fd->node->datain_sig, &sock->mtx);
		}

		r = (data->fd >= 0) ? E_OK : -errno;
		break;

	default:
		r = -E_NOSUP;
	}

	mutex_unlock(&sock->mtx);

	return r;
}

static int connect(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	if(sock->dev != 0x0)
		return_errno(E_CONN);

	if(netdev_bind(sock, addr, addr_len) != 0)
		return -errno;

	return sock->dev->ops.connect(sock);
}

static int bind(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	if(sock->dev != 0x0)
		return_errno(E_CONN);

	if(netdev_bind(sock, addr, addr_len) != 0)
		return -errno;

	return sock->dev->ops.bind(sock);
}

static int listen(socket_t *sock, int backlog){
	netdev_t *dev;


	dev = sock->dev;

	if(sock->type != SOCK_STREAM)
		return_errno(E_INVAL);

	if(dev == 0x0 || !socket_connected(sock))
		return_errno(E_NOCONN);

	if(socket_listen(sock, backlog) != 0)
		return -errno;

	return dev->ops.listen(sock);
}

static int accept(socket_t *sock, sock_addr_t *addr, size_t *addr_len){
	int fd_id;
	socket_t *client;


	if(sock->type != SOCK_STREAM)
		return_errno(E_INVAL);

	if(sock->dev == 0x0 || !socket_connected(sock))
		return_errno(E_NOCONN);

	/* get client socket */
	mutex_lock(&sock->mtx);

	client = socket_get_client_addr(sock, addr, addr_len);

	/* create netfs node */
	fd_id = -1;

	if(client){
		fd_id = link_to_fs(client, sched_running()->parent);

		if(fd_id < 0)
			(void)socket_add_client_socket(sock, client, false);
	}

	mutex_unlock(&sock->mtx);

	return fd_id;
}

static size_t recvfrom(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len){
	size_t n;
	socket_t *sock;
	datagram_t *dgram;


	n = 0;
	sock = (socket_t*)fd->node->data;

	mutex_lock(&sock->mtx);

	if(sock->dev == 0x0 || !socket_connected(sock))
		goto_errno(end, E_NOCONN);

	if(sock->type == SOCK_DGRAM){
		dgram = list_first(sock->dgrams);

		if(dgram == 0x0)
			goto end;

		if(addr_len && addr){
			memcpy(addr, &dgram->addr, *addr_len);
			*addr_len = net_domain_cfg[sock->addr.domain].addr_len;
		}

		n = MIN(data_len, dgram->len - dgram->idx);
		memcpy(data, dgram->data + dgram->idx, n);
		dgram->idx += n;

		if(dgram->idx == dgram->len){
			list_rm(sock->dgrams, dgram);
			kfree(dgram->data);
			kfree(dgram);
		}
	}
	else
		n = ringbuf_read(&sock->stream, data, data_len);

end:
	mutex_unlock(&sock->mtx);

	return n;
}

static ssize_t sendto(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len){
	int n;
	socket_t *sock;
	netdev_t *dev;


	n = 0;
	sock = (socket_t*)fd->node->data;

	mutex_lock(&sock->mtx);

	dev = sock->dev;

	if(sock->type == SOCK_STREAM){
		if(dev == 0x0 || !socket_connected(sock))
			goto_errno(end, E_NOCONN);

		// NOTE addr != 0x0 are ignored, instead of returning E_CONN
	}
	else{
		if(addr){
			if(netdev_bind(sock, addr, addr_len) != 0)
				goto end;
		}
		else if(dev == 0x0 || !socket_connected(sock))
			goto_errno(end, E_NOCONN);

		dev = sock->dev;
	}

	n = dev->ops.send(sock, data, data_len);

end:
	mutex_unlock(&sock->mtx);

	return n;
}

static int free_client(socket_t *sock){
	if(sock == 0x0)
		return -1;

	if(sock->dev)
		sock->dev->ops.close(sock);

	socket_free(sock);

	return 0;
}

static int link_to_fs(socket_t *sock, process_t *this_p){
	static uint8_t sock_id = 0;
	char name[4];
	fs_node_t *node;
	fs_filed_t *fd;


	/* create netfs node */
	mutex_lock(&netfs_mtx);
	sock_id++;
	mutex_unlock(&netfs_mtx);

	if(sock_id == 0)
		return_errno(E_LIMIT);

	itoa(sock_id, 10, name, 4);

	node = fs_node_create(netfs_root, name, strlen(name), FT_REG, sock, netfs_id);

	if(node == 0x0)
		goto err_0;

	sock->node = node;

	/* create file descriptor */
	fd = fs_fd_alloc(node, this_p, O_RDWR, 0x0);

	if(fd == 0x0)
		goto err_1;

	 return fd->id;


err_1:
	fs_node_destroy(node);

err_0:
	return -errno;
}
