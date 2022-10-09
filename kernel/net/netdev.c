/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/memory.h>
#include <kernel/net.h>
#include <kernel/inet.h>
#include <kernel/devfs.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/mutex.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/net.h>
#include <sys/inet.h>


/* local/static prototypes */
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);


/* global variables */
net_domain_cfg_t const net_domain_cfg[] = {
	{ sizeof(inetdev_cfg_t), sizeof(sock_addr_inet_t), inet_match_addr },	// AF_INET
};


/* static variables */
static netdev_t *dev_lst = 0x0;
static mutex_t net_mtx = MUTEX_INITIALISER();


/* global functions */
devfs_dev_t *netdev_register(char const *name, net_family_t domain, netdev_itf_t *itf, void *payload){
	devfs_dev_t *dev;
	devfs_ops_t dev_ops;
	netdev_t *netdev;


	if(!callbacks_set(itf, netdev_itf_t))
		goto_errno(err_0, E_INVAL);

	/* init netdev */
	netdev = kcalloc(1, sizeof(netdev_t));

	if(netdev == 0x0)
		goto err_0;

	netdev->domain = domain;
	netdev->hw = *itf;
	netdev->payload = payload;

	/* register device */
	dev_ops.open = 0x0;
	dev_ops.close = 0x0;
	dev_ops.read = 0x0;
	dev_ops.write = 0x0;
	dev_ops.ioctl = ioctl;
	dev_ops.fcntl = 0x0;
	dev_ops.mmap = 0x0;

	dev = devfs_dev_register(name, &dev_ops, netdev);

	if(dev == 0x0)
		goto err_1;

	list_add_tail_safe(dev_lst, netdev, &net_mtx);

	return dev;


err_1:
	kfree(netdev);

err_0:
	return 0x0;
}

int netdev_release(devfs_dev_t *dev){
	netdev_t *netdev;


	netdev = dev->payload;

	if(devfs_dev_release(dev) != 0)
		return -errno;

	list_rm_safe(dev_lst, netdev, &net_mtx);
	kfree(netdev);

	return 0;
}

netdev_t *netdev_bind(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	netdev_t *dev;


	if(addr_len != net_domain_cfg[sock->addr.domain].addr_len)
		goto_errno(err, E_INVAL);

	mutex_lock(&net_mtx);

	list_for_each(dev_lst, dev){
		if(dev->domain != addr->domain)
			continue;

		if(net_domain_cfg[addr->domain].match_addr(dev, addr, addr_len))
			break;
	}

	mutex_unlock(&net_mtx);

	if(dev == 0x0)
		goto_errno(err, E_END);

	socket_bind(sock, dev, addr, addr_len);

	return dev;


err:
	return 0x0;
}


/* local functions */
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	size_t cfg_size;
	netdev_t *netdev;


	netdev = dev->payload;
	cfg_size = net_domain_cfg[netdev->domain].cfg_size;

	if(n != cfg_size)
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(arg, netdev->hw.cfg, cfg_size);
		return 0;

	case IOCTL_CFGWR:
		memcpy(netdev->hw.cfg, arg, cfg_size);

		if(netdev->hw.configure(netdev) != 0)
			return -errno;

		return 0;

	default:
		return_errno(E_NOSUP);
	}

	return 0;
}
