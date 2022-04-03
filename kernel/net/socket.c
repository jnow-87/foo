/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <kernel/ksignal.h>
#include <kernel/sched.h>
#include <kernel/net.h>
#include <sys/types.h>
#include <sys/net.h>
#include <sys/string.h>
#include <sys/ringbuf.h>
#include <sys/list.h>
#include <sys/mutex.h>
#include <sys/math.h>


/* local/static prototypes */
static void notify_node(fs_node_t *node);


/* global functions */
socket_t *socket_alloc(sock_type_t type, net_family_t domain, size_t addr_len){
	void *rx_data;
	size_t size;
	socket_t *sock;


	size = sizeof(socket_t) + addr_len - sizeof(sock_addr_t);	// NOTE substract sizeof(sock_addr_t) since
																//		it is included in socket_t and addr_len

	sock = kcalloc(1, size);

	if(sock == 0x0)
		goto err_0;

	sock->type = type;
	sock->addr.domain = domain;

	mutex_init(&sock->mtx, MTX_NESTED);
	ringbuf_init(&sock->clients, 0x0, 0);

	if(type == SOCK_STREAM){
		rx_data = kmalloc(CONFIG_NET_RXBUF_SIZE);

		if(rx_data == 0x0)
			goto err_1;

		ringbuf_init(&sock->stream, rx_data, CONFIG_NET_RXBUF_SIZE);
	}

	return sock;


err_1:
	kfree(sock);

err_0:
	return 0x0;
}

void socket_free(socket_t *sock){
	datagram_t *dgram;


	if(sock->type == SOCK_DGRAM){
		while(1){
			dgram = list_first(sock->dgrams);

			if(dgram == 0x0)
				break;

			list_rm(sock->dgrams, dgram);
			kfree(dgram->data);
			kfree(dgram);
		}
	}
	else{
		kfree(sock->clients.data);
		kfree(sock->stream.data);
	}

	kfree(sock);
}

int socket_listen(socket_t *sock, int backlog){
	void *data;
	size_t size;


	size = sizeof(socket_t*) * backlog + 1;
	data = kmalloc(size);

	if(data == 0x0)
		return -errno;

	mutex_lock(&sock->mtx);
	ringbuf_init(&sock->clients, data, size);
	mutex_unlock(&sock->mtx);

	return E_OK;
}

void socket_link(socket_t *sock, fs_node_t *node){
	mutex_lock(&sock->mtx);
	sock->node = node;
	mutex_unlock(&sock->mtx);
}

void socket_unlink(socket_t *sock){
	fs_node_t *node;


	mutex_lock(&sock->mtx);

	node = sock->node;
	sock->node = 0x0;

	notify_node(node);

	mutex_unlock(&sock->mtx);
}

fs_node_t *socket_linked(socket_t *sock){
	fs_node_t *node;


	mutex_lock(&sock->mtx);
	node = sock->node;
	mutex_unlock(&sock->mtx);

	return node;
}

void socket_bind(socket_t *sock, netdev_t *dev, sock_addr_t *addr, size_t addr_len){
	mutex_lock(&sock->mtx);

	sock->dev = dev;
	memcpy(&sock->addr, addr, addr_len);

	mutex_unlock(&sock->mtx);
}

netdev_t *socket_bound(socket_t *sock){
	netdev_t *dev;


	mutex_lock(&sock->mtx);
	dev = sock->dev;
	mutex_unlock(&sock->mtx);

	return dev;
}

socket_t *socket_add_client_socket(socket_t *sock, socket_t *client, bool notify){
	fs_node_t *node;


	mutex_lock(&sock->mtx);

	if(ringbuf_left(&sock->clients) < sizeof(socket_t*))
		goto_errno(err, E_LIMIT);

	node = sock->node;
	(void)ringbuf_write(&sock->clients, &client, sizeof(socket_t*));

	if(notify)
		notify_node(node);

	mutex_unlock(&sock->mtx);

	return client;


err:
	mutex_unlock(&sock->mtx);

	return 0x0;
}

socket_t *socket_add_client_addr(socket_t *sock, sock_addr_t *addr, size_t addr_len, bool notify){
	socket_t *client;


	client = socket_alloc(SOCK_STREAM, addr->domain, addr_len);

	if(client == 0x0)
		return 0x0;

	mutex_lock(&sock->mtx);
	client->dev = sock->dev;
	mutex_unlock(&sock->mtx);

	memcpy(&client->addr, addr, addr_len);

	if(socket_add_client_socket(sock, client, notify) != client){
		socket_free(client);

		return 0x0;
	}

	return client;
}

socket_t *socket_get_client_socket(socket_t *sock){
	socket_t *client;


	client = 0x0;

	mutex_lock(&sock->mtx);

	if(ringbuf_contains(&sock->clients) >= sizeof(socket_t*))
		(void)ringbuf_read(&sock->clients, &client, sizeof(socket_t*));

	mutex_unlock(&sock->mtx);

	return client;
}

socket_t *socket_get_client_addr(socket_t *sock, sock_addr_t *addr, size_t *addr_len){
	socket_t *client;


	client = socket_get_client_socket(sock);

	if(client && addr_len && addr){
		memcpy(addr, &client->addr, *addr_len);
		*addr_len = net_domain_cfg[client->addr.domain].addr_len;
	}

	return client;
}

int socket_datain_stream(socket_t *sock, uint8_t *data, size_t len, bool signal){
	size_t n;
	fs_node_t *node;


	n = 0;

	mutex_lock(&sock->mtx);

	while(1){
		n += ringbuf_write(&sock->stream, data + n, len - n);
		node = sock->node;

		if(n == len)
			break;

		if(node == 0x0)
			break;

		notify_node(node);

		mutex_unlock(&sock->mtx);
		sched_yield();
		mutex_lock(&sock->mtx);
	}

	if(signal)
		notify_node(node);

	mutex_unlock(&sock->mtx);

	if(node == 0x0)
		return_errno(E_LIMIT);

	return E_OK;
}

int socket_dataout_stream(socket_t *sock, void *data, size_t data_len){
	size_t n;


	mutex_lock(&sock->mtx);
	n = ringbuf_read(&sock->stream, data, data_len);
	mutex_unlock(&sock->mtx);

	return n;
}

int socket_datain_dgram(socket_t *sock, sock_addr_t *addr, size_t addr_len, uint8_t *data, size_t len){
	datagram_t *dgram;
	fs_node_t *node;


	dgram = kmalloc(sizeof(datagram_t) + addr_len - sizeof(sock_addr_t));

	if(dgram == 0x0)
		return -errno;

	dgram->idx = 0;
	dgram->len = len;
	dgram->data = data;

	memcpy(&dgram->addr, addr, addr_len);

	mutex_lock(&sock->mtx);

	list_add_tail(sock->dgrams, dgram);
	node = sock->node;

	notify_node(node);

	mutex_unlock(&sock->mtx);

	return E_OK;
}

int socket_dataout_dgram(socket_t *sock, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len){
	size_t n;
	datagram_t *dgram;


	mutex_lock(&sock->mtx);

	n = 0;
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

end:
	mutex_unlock(&sock->mtx);

	return n;
}


/* local functions */
static void notify_node(fs_node_t *node){
	if(node == 0x0)
		return;

	mutex_lock(&node->mtx);
	ksignal_send(&node->datain_sig);
	mutex_unlock(&node->mtx);
}
