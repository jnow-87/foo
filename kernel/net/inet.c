/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/net.h>
#include <sys/types.h>
#include <sys/inet.h>


/* global functions */
bool inet_match_addr(netdev_t *dev, sock_addr_t *addr, size_t addr_len){
	inetdev_cfg_t *cfg;
	sock_addr_inet_t *inet_addr;


	cfg = (inetdev_cfg_t*)dev->hw.cfg;
	inet_addr = (sock_addr_inet_t*)addr;

	if(addr_len != sizeof(sock_addr_inet_t))
		return false;

	if(inet_addr->data.addr == INET_ADDR_ANY)
		return true;

	if((cfg->ip & cfg->netmask) == (inet_addr->data.addr & cfg->netmask))
		return true;

	return false;
}
