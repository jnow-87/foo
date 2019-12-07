/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_INET_H
#define KERNEL_INET_H


#include <kernel/net.h>
#include <sys/types.h>
#include <sys/net.h>


/* prototypes */
bool inet_match_addr(netdev_t *dev, sock_addr_t *addr, size_t addr_len);


#endif // KERNEL_INET_H
