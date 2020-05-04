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
	mutex_t mtx;

	struct netdev_t *dev;
	fs_node_t *node;

	ringbuf_t stream;
	datagram_t *dgrams;

	ringbuf_t clients;

	sock_type_t type;
	sock_addr_t addr;		// NOTE addr has to be the last member of this
							// struct since it has a flexible array member
} socket_t;

typedef struct{
	int (*configure)(struct netdev_t *dev);

	int (*connect)(socket_t *sock);
	int (*bind)(socket_t *sock);
	int (*listen)(socket_t *sock);
	void (*close)(socket_t *sock);

	ssize_t (*send)(socket_t *sock, void *data, size_t data_len);

	void *cfg;
} netdev_itf_t;

typedef struct netdev_t{
	struct netdev_t *prev,
					*next;

	net_family_t domain;

	netdev_itf_t hw;
	void *data;
} netdev_t;

typedef struct{
	size_t cfg_size,
		   addr_len;

	bool (*match_addr)(netdev_t *dev, sock_addr_t *addr, size_t addr_len);
} net_domain_cfg_t;


/* prototypes */
// network device function
devfs_dev_t *netdev_register(char const *name, net_family_t domain, netdev_itf_t *itf, void *data);
int netdev_release(devfs_dev_t *dev);

netdev_t *netdev_bind(socket_t *sock, sock_addr_t *addr, size_t addr_len);

// socket function
socket_t *socket_alloc(sock_type_t type, net_family_t domain, size_t addr_len);
void socket_free(socket_t *sock);

int socket_listen(socket_t *sock, int backlog);

void socket_link(socket_t *sock, fs_node_t *node);
void socket_unlink(socket_t *sock);
fs_node_t *socket_linked(socket_t *sock);

void socket_bind(socket_t *sock, netdev_t *dev, sock_addr_t *addr, size_t addr_len);
netdev_t *socket_bound(socket_t *sock);

socket_t *socket_add_client_socket(socket_t *sock, socket_t *client, bool notify);
socket_t *socket_add_client_addr(socket_t *sock, sock_addr_t *addr, size_t addr_len, bool notify);
socket_t *socket_get_client_socket(socket_t *sock);
socket_t *socket_get_client_addr(socket_t *sock, sock_addr_t *addr, size_t *addr_len);

int socket_datain_stream(socket_t *sock, uint8_t *data, size_t len, bool signal);
int socket_dataout_stream(socket_t *sock, void *data, size_t data_len);
int socket_datain_dgram(socket_t *sock, sock_addr_t *addr, size_t addr_len, uint8_t *data, size_t len);
int socket_dataout_dgram(socket_t *sock, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len);


/* external variables */
extern net_domain_cfg_t const net_domain_cfg[];


#endif // KERNEL_NET_H
