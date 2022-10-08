/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_NET_H
#define SYS_NET_H


#include <sys/types.h>


/* types */
typedef enum{
	AF_INET = 0,
} net_family_t;

typedef enum{
	SOCK_STREAM = 1,
	SOCK_DGRAM,
} sock_type_t;

typedef struct{
	net_family_t domain;
	uint8_t data[];
} sock_addr_t;

typedef struct{
	int fd;
	int backlog;

	size_t addr_len;
	sock_addr_t addr;		// NOTE addr has to be the last member of this
							// struct since it has a flexible array member
} socket_ioctl_t;


#endif // SYS_NET_H
