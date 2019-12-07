/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <kernel/ksignal.h>
#include <kernel/net.h>
#include <sys/types.h>
#include <sys/net.h>
#include <sys/string.h>
#include <sys/ringbuf.h>
#include <sys/list.h>


/* global functions */
socket_t *socket_alloc(sock_type_t type, net_family_t domain, size_t addr_len){
	void *rx_data;
	size_t size;
	socket_t *sock;


	size = sizeof(socket_t) + addr_len - sizeof(sock_addr_t);	// NOTE substract sizeof(sock_addr_t) since
																//		it is included in socket_t and addr_len

	sock = kmalloc(size);

	if(sock == 0x0)
		goto err_0;

	memset(sock, 0x0, size);

	sock->type = type;
	sock->addr.domain = domain;
	sock->mtx = 0x0;

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
	if(sock->type == SOCK_STREAM)
		kfree(sock->stream.data);

	kfree(sock);
}

int socket_datain(socket_t *sock, sock_addr_t *addr, size_t addr_len, uint8_t *data, size_t len){
	datagram_t *dgram;
	fs_node_t *node;


	if(sock->type == SOCK_DGRAM){
		dgram = kmalloc(sizeof(datagram_t) + len + addr_len - sizeof(sock_addr_t));

		if(dgram == 0x0)
			goto err;

		dgram->idx = 0;
		dgram->len = len;
		dgram->data = (uint8_t*)(&dgram->addr + addr_len);

		memcpy(dgram->data, data, len);
		memcpy(&dgram->addr, &addr, addr_len);

		list_add_tail(sock->dgrams, dgram);
	}
	else{
		if(ringbuf_write(&sock->stream, data, len) != len)
			goto_errno(err, E_LIMIT);

	}

	node = netfs_get_fsnode(sock);
	ksignal_send(&node->rd_sig);

	return E_OK;


err:
	return -errno;
}

int socket_add_client(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	socket_t *client;


	client = socket_alloc(SOCK_STREAM, addr->domain, addr_len);

	if(client == 0x0)
		return -errno;

	memcpy(&client->addr, addr, addr_len);
	list_add_tail(sock, client);

	return E_OK;
}

void socket_disconnect(socket_t *sock){
	fs_node_t *node;


	node = netfs_get_fsnode(sock);
	node->data = 0x0;
}
