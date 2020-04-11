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


	p.type = type;
	p.addr->domain = domain;

	if(sc(SC_SOCKET, &p) != E_OK)
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

	return ioctl(fd, IOCTL_CONNECT, p, size);
}

int bind(int fd, sock_addr_t *addr, size_t addr_len){
	size_t size = sizeof(socket_ioctl_t) + addr_len - sizeof(sock_addr_t);
	char _p[size];
	socket_ioctl_t *p;


	p = (socket_ioctl_t*)_p;

	p->addr_len = addr_len;
	memcpy(&p->addr, addr, addr_len);

	return ioctl(fd, IOCTL_BIND, p, size);
}

int listen(int fd, int backlog){
	return ioctl(fd, IOCTL_LISTEN, &backlog, sizeof(int*));
}

int accept(int fd, sock_addr_t *addr, size_t *addr_len){
	size_t size = sizeof(socket_ioctl_t) + *addr_len - sizeof(sock_addr_t);
	char _p[size];
	socket_ioctl_t *p;


	p = (socket_ioctl_t*)_p;

	p->addr_len = *addr_len;

	if(ioctl(fd, IOCTL_ACCEPT, p, size) != E_OK)
		return -1;

	*addr_len = p->addr_len;
	memcpy(addr, &p->addr, p->addr_len);

	return p->fd;
}

ssize_t recv(int fd, void *data, size_t data_len){
	return recvfrom(fd, data, data_len, 0x0, 0x0);
}

ssize_t recvfrom(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len){
	sc_socket_t p;


	p.fd = fd;
	p.data = data;
	p.data_len = data_len;
	p.addr = addr;
	p.addr_len = addr_len ? *addr_len : 0;

	if(sc(SC_RECV, &p) != E_OK)
		return -1;

	if(addr_len)
		*addr_len = p.addr_len;

	return p.data_len;
}

ssize_t send(int fd, void *data, size_t data_len){
	return sendto(fd, data, data_len, 0x0, 0);
}

ssize_t sendto(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len){
	sc_socket_t p;


	p.fd = fd;
	p.data = data;
	p.data_len = data_len;
	p.addr = addr;
	p.addr_len = addr_len;

	if(sc(SC_SEND, &p) != E_OK)
		return -1;
	return p.data_len;
}
