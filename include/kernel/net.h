/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_NET_H
#define KERNEL_NET_H


#include <kernel/devfs.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/ringbuf.h>
#include <sys/net.h>
#include <sys/inet.h>


/* incomplete types */
struct netdev_t;


/* types */
typedef struct datagram_t{
	struct datagram_t *prev,
					  *next;

	size_t len,
		   idx;
	uint8_t *data;

	sock_addr_t addr;		// NOTE addr has to be the last member of this
							// struct since it has a flexible array member
} datagram_t;

typedef struct socket_t{
	struct socket_t *prev,
					*next;

	mutex_t *mtx;

	struct netdev_t *dev;

	ringbuf_t stream;
	datagram_t *dgrams;

	struct socket_t *clients;

	sock_type_t type;
	sock_addr_t addr;		// NOTE addr has to be the last member of this
							// struct since it has a flexible array member
} socket_t;

typedef struct{
	int (*configure)(struct netdev_t *dev, void *cfg);

	int (*connect)(socket_t *sock);
	int (*listen)(socket_t *sock, int backlog);
	int (*close)(socket_t *sock);

	ssize_t (*send)(socket_t *sock, void *data, size_t data_len);
} netdev_ops_t;

typedef struct netdev_t{
	struct netdev_t *prev,
					*next;

	net_family_t domain;

	netdev_ops_t ops;
	void *data;
	uint8_t cfg[];
} netdev_t;

typedef struct{
	size_t cfg_size,
		   addr_len;

	bool (*match_addr)(netdev_t *dev, sock_addr_t *addr, size_t addr_len);
} net_domain_cfg_t;


/* prototypes */
// netfs functions
socket_t *netfs_get_socket(fs_filed_t *fd);
fs_node_t *netfs_get_fsnode(socket_t *sock);

// network device function
devfs_dev_t *netdev_register(char const *name, netdev_ops_t *ops, net_family_t domain, void *data);
int netdev_release(devfs_dev_t *dev);

int netdev_bind(socket_t *sock, sock_addr_t *addr, size_t addr_len);

// socket function
socket_t *socket_alloc(sock_type_t type, net_family_t domain, size_t addr_len);
void socket_free(socket_t *sock);

int socket_datain(socket_t *sock, sock_addr_t *addr, size_t addr_len, uint8_t *data, size_t len);
int socket_add_client(socket_t *sock, sock_addr_t *addr, size_t addr_len);
void socket_disconnect(socket_t *sock);


/* external variables */
extern net_domain_cfg_t const net_domain_cfg[];


#endif // KERNEL_NET_H
