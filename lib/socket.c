/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <lib/unistd.h>
#include <lib/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/net.h>
#include <sys/string.h>


/* global functions */
int socket(net_family_t domain, sock_type_t type){
	sc_socket_t p;
	sock_addr_t addr;


	addr.domain = domain;
	p.type = type;
	p.addr = &addr;

	if(sc(SC_SOCKET, &p) != 0)
		return -1;

	return p.fd;
}

int connect(int fd, sock_addr_t *addr, size_t addr_len){
	size_t size = sizeof(socket_ioctl_t) + addr_len - sizeof(sock_addr_t);
	char _p[size];
	socket_ioctl_t *p;


	p = (socket_ioctl_t*)_p;

	p->addr_len = addr_len;
	memcpy(&p->addr, addr, addr_len);

	return ionctl(fd, IOCTL_CONNECT, p, size);
}

int bind(int fd, sock_addr_t *addr, size_t addr_len){
	size_t size = sizeof(socket_ioctl_t) + addr_len - sizeof(sock_addr_t);
	char _p[size];
	socket_ioctl_t *p;


	p = (socket_ioctl_t*)_p;

	p->addr_len = addr_len;
	memcpy(&p->addr, addr, addr_len);

	return ionctl(fd, IOCTL_BIND, p, size);
}

int listen(int fd, int backlog){
	socket_ioctl_t p;


	p.backlog = backlog;
	p.addr_len = 0;

	return ioctl(fd, IOCTL_LISTEN, &p);
}

int accept(int fd, sock_addr_t *addr, size_t *addr_len){
	size_t size = sizeof(socket_ioctl_t) + *addr_len - sizeof(sock_addr_t);
	char _p[size];
	socket_ioctl_t *p;


	p = (socket_ioctl_t*)_p;

	p->addr_len = *addr_len;

	if(ionctl(fd, IOCTL_ACCEPT, p, size) != 0)
		return -1;

	*addr_len = p->addr_len;
	memcpy(addr, &p->addr, p->addr_len);

	return p->fd;
}

ssize_t recv(int fd, void *buf, size_t n){
	return recvfrom(fd, buf, n, 0x0, 0x0);
}

ssize_t recvfrom(int fd, void *buf, size_t n, sock_addr_t *addr, size_t *addr_len){
	sc_socket_t p;


	p.fd = fd;
	p.buf = buf;
	p.buf_len = n;
	p.addr = addr;
	p.addr_len = addr_len ? *addr_len : 0;

	if(sc(SC_RECV, &p) != 0)
		return -1;

	if(addr_len)
		*addr_len = p.addr_len;

	return p.buf_len;
}

ssize_t send(int fd, void *buf, size_t n){
	return sendto(fd, buf, n, 0x0, 0);
}

ssize_t sendto(int fd, void *buf, size_t n, sock_addr_t *addr, size_t addr_len){
	sc_socket_t p;


	p.fd = fd;
	p.buf = buf;
	p.buf_len = n;
	p.addr = addr;
	p.addr_len = addr_len;

	if(sc(SC_SEND, &p) != 0)
		return -1;

	return p.buf_len;
}
