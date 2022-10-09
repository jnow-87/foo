/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_SOCKET_H
#define LIB_SOCKET_H


#include <config/config.h>
#include <sys/types.h>
#include <sys/compiler.h>
#include <sys/net.h>


/* prototypes */
int socket(net_family_t domain, sock_type_t type);

int connect(int fd, sock_addr_t *addr, size_t addr_len);

int bind(int fd, sock_addr_t *addr, size_t addr_len);
int listen(int fd, int backlog);
int accept(int fd, sock_addr_t *addr, size_t *addr_len);

ssize_t recv(int fd, void *buf, size_t n);
ssize_t recvfrom(int fd, void *buf, size_t n, sock_addr_t *addr, size_t *addr_len);
ssize_t send(int fd, void *buf, size_t n);
ssize_t sendto(int fd, void *buf, size_t n, sock_addr_t *addr, size_t addr_len);


/* disabled-call macros */
#ifndef CONFIG_SC_SOCKET
# define socket(domain, type)					CALL_DISABLED(socket, CONFIG_SC_SOCKET)
# define connect(fd, addr, add_len)				CALL_DISABLED(connect, CONFIG_SC_SOCKET)
# define bind(fd, addr, addr_len)				CALL_DISABLED(bind, CONFIG_SC_SOCKET)
# define listen(fd, backlog)					CALL_DISABLED(listen, CONFIG_SC_SOCKET)
# define accept(fd, addr, addr_len)				CALL_DISABLED(accpect, CONFIG_SC_SOCKET)
# define recv(fd, buf, n)						CALL_DISABLED(recv, CONFIG_SC_SOCKET)
# define recvfrom(fd, buf, n, addr, addr_len)	CALL_DISABLED(recvfrom, CONFIG_SC_SOCKET)
# define send(fd, buf, n)						CALL_DISABLED(send, CONFIG_SC_SOCKET)
# define sendto(fd, buf, n, addr, addr_len)		CALL_DISABLED(sendto, CONFIG_SC_SOCKET)
#endif // CONFIG_SC_SOCKET


#endif // LIB_SOCKET_H
