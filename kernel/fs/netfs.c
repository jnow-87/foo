/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/memory.h>
#include <arch/atomic.h>
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

static fs_filed_t *create_filed(process_t *this_p);


/* static variables */
static int netfs_id = 0;
static fs_node_t *netfs_root = 0x0;


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
	fs_filed_t *fd;


	p = (sc_socket_t*)_p;
	this_p = sched_running()->parent;

	/* allocate socket */
	copy_from_user(&addr, p->addr, sizeof(sock_addr_t), this_p);

	sock = socket_alloc(p->type, addr.domain, net_domain_cfg[addr.domain].addr_len);

	if(sock == 0x0)
		goto err_0;

	/* create netfs node */
	fd = create_filed(this_p);

	if(fd == 0x0)
		goto err_1;

	fd->node->data = sock;
	socket_link(sock, fd->node);

	p->fd = fd->id;

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
	netdev_t *dev;


	fs_lock();

	sock = (socket_t*)fd->node->data;
	dev = socket_bound(sock);

	if(dev)
		dev->hw.close(sock);

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

	return -errno;
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

			ksignal_wait(&fd->node->datain_sig);
		}

		r = (data->fd >= 0) ? E_OK : -errno;
		break;

	default:
		r = -E_NOSUP;
	}

	return r;
}

static int connect(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	netdev_t *dev;


	if(socket_bound(sock) != 0x0)
		return_errno(E_CONN);

	dev = netdev_bind(sock, addr, addr_len);

	if(dev == 0x0)
		return -errno;

	return dev->hw.connect(sock);
}

static int bind(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	netdev_t *dev;


	if(socket_bound(sock) != 0x0)
		return_errno(E_CONN);

	dev = netdev_bind(sock, addr, addr_len);

	if(dev == 0x0)
		return -errno;

	return dev->hw.bind(sock);
}

static int listen(socket_t *sock, int backlog){
	netdev_t *dev;


	dev = socket_bound(sock);

	if(sock->type != SOCK_STREAM)
		return_errno(E_INVAL);

	if(dev == 0x0 || socket_linked(sock) == 0x0)
		return_errno(E_NOCONN);

	if(socket_listen(sock, backlog) != 0)
		return -errno;

	return dev->hw.listen(sock);
}

static int accept(socket_t *sock, sock_addr_t *addr, size_t *addr_len){
	socket_t *client;
	fs_filed_t *fd;
	process_t *this_p;


	if(sock->type != SOCK_STREAM)
		return_errno(E_INVAL);

	if(socket_bound(sock) == 0x0 || socket_linked(sock) == 0x0)
		return_errno(E_NOCONN);

	this_p = sched_running()->parent;
	fd = create_filed(this_p);

	if(fd == 0x0)
		return -1;

	client = socket_get_client_addr(sock, addr, addr_len);

	if(client == 0x0){
		fs_fd_free(fd, this_p);
		fs_node_destroy(fd->node);

		return -1;
	}

	fd->node->data = client;
	socket_link(client, fd->node);

	return fd->id;
}

static size_t recvfrom(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len){
	socket_t *sock;


	sock = (socket_t*)fd->node->data;

	if(socket_bound(sock) == 0x0 || socket_linked(sock) == 0x0){
		errno = E_NOCONN;
		return 0;
	}

	if(sock->type == SOCK_DGRAM)
		return socket_dataout_dgram(sock, data, data_len, addr, addr_len);
	return socket_dataout_stream(sock, data, data_len);
}

static ssize_t sendto(fs_filed_t *fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len){
	socket_t *sock;
	netdev_t *dev;


	sock = (socket_t*)fd->node->data;
	dev = socket_bound(sock);;

	if(sock->type == SOCK_DGRAM){
		if(addr){
			dev = netdev_bind(sock, addr, addr_len);

			if(dev == 0x0)
				return 0;
		}
		else if(dev == 0x0 || socket_linked(sock) == 0x0)
			goto_errno(err, E_NOCONN);
	}
	else{
		if(dev == 0x0 || socket_linked(sock) == 0x0)
			goto_errno(err, E_NOCONN);

		// NOTE addr != 0x0 is ignored, instead of returning E_CONN
	}

	return dev->hw.send(sock, data, data_len);


err:
	return 0;
}

static int free_client(socket_t *sock){
	netdev_t *dev;


	if(sock == 0x0)
		return -1;

	dev = socket_bound(sock);

	if(dev)
		dev->hw.close(sock);

	socket_free(sock);

	return 0;
}

static fs_filed_t *create_filed(process_t *this_p){
	static int sock_id = 0;
	char name[4];
	fs_node_t *node;
	fs_filed_t *fd;


	/* create netfs node */
	atomic_inc(&sock_id, 1);

	if(sock_id == 0)
		goto_errno(err_0, E_LIMIT);

	itoa(sock_id, 10, name, 4);

	node = fs_node_create(netfs_root, name, strlen(name), FT_REG, 0x0, netfs_id);

	if(node == 0x0)
		goto err_0;

	/* create file descriptor */
	fd = fs_fd_alloc(node, this_p, O_RDWR, 0x0);

	if(fd == 0x0)
		goto err_1;

	 return fd;


err_1:
	fs_node_destroy(node);

err_0:
	atomic_inc(&sock_id, -1);

	return 0x0;
}
