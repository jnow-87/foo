/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/avr/atmega.h>
#include <arch/interrupt.h>
#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/devfs.h>
#include <kernel/thread.h>
#include <kernel/usignal.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <sys/register.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/list.h>
#include <sys/mutex.h>


/* types */
typedef enum{
	PORT_IN = 0x1,
	PORT_OUT = 0x2,
	PORT_INOUT = 0x3,
} port_dir_t;

typedef struct sig_tgt_t{
	struct sig_tgt_t *prev,
					 *next;

	signal_t signal;
	thread_t *thread;
	fs_filed_t *fd;
} sig_tgt_t;

typedef struct{
	uint8_t volatile pin,
					 ddr,
					 port;
} gpio_regs_t;

typedef struct{
	// port registers
	gpio_regs_t *regs;

	// interrupt registers
	uint8_t volatile *pcicr,
					 *pcmsk;

	uint8_t const pcint_num;

	// configuration
	uint8_t const dir,		// direction, cf. port_dir_t
				  mask;		// pin bits
} dt_data_t;

typedef struct{
	dt_data_t *dtd;

	sig_tgt_t *sig_tgt_lst;
	mutex_t mtx;
} dev_data_t;


/* local/static prototypes */
static int close(struct devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);

static void int_hdlr(int_num_t num, void *data);

static uint8_t rightmost_bit(uint8_t v);

/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd;
	devfs_ops_t ops;
	devfs_dev_t *dev;
	dev_data_t *port;
	gpio_regs_t *regs;


	dtd = (dt_data_t*)dt_data;
	regs = dtd->regs;

	/* register device */
	ops.open = 0x0;
	ops.close = close;
	ops.read = dtd->dir & PORT_IN ? read : 0x0;
	ops.write = dtd->dir & PORT_OUT ? write : 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;

	port = kmalloc(sizeof(dev_data_t));

	if(port == 0x0)
		goto err_0;

	port->dtd = dt_data;
	port->sig_tgt_lst = 0x0;

	mutex_init(&port->mtx, MTX_NOINT);

	dev = devfs_dev_register(name, &ops, 0x0, port);

	if(dev == 0x0)
		goto err_1;

	/* configure port */
	regs->port |= dtd->mask;

	if(dtd->dir & PORT_OUT)
		regs->ddr |= dtd->mask;

	/* configure interrupt */
	if(dtd->pcicr != 0x0 && dtd->pcmsk != 0x0){
		*dtd->pcicr |= (0x1 << (dtd->pcint_num / 8));
		*dtd->pcmsk |= dtd->mask;

		if(int_register(INT_PCINT0 + dtd->pcint_num / 8, int_hdlr, dev))
			goto err_2;
	}

	return E_OK;


err_2:
	devfs_dev_release(dev);

err_1:
	kfree(port);

err_0:
	return -errno;
}

device_probe("avr,gpio", probe);

static int close(struct devfs_dev_t *dev, fs_filed_t *fd){
	dev_data_t *port;
	sig_tgt_t *tgt;


	port = (dev_data_t*)dev->data;

	list_for_each(port->sig_tgt_lst, tgt){
		if(tgt->fd != fd)
			continue;

		DEBUG("remove %s.%d from signal list for \"%s\"\n", tgt->thread->parent->name, tgt->thread->tid, dev->node->name);

		mutex_lock(&port->mtx);
		list_rm(port->sig_tgt_lst, tgt);
		mutex_unlock(&port->mtx);

		kfree(tgt);
	}

	return E_OK;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dt_data_t *dtd;
	uint8_t v;


	dtd = ((dev_data_t*)dev->data)->dtd;

	/* read port */
	v = dtd->regs->pin;
	*((char*)buf) = (v & dtd->mask) >> rightmost_bit(dtd->mask);

	DEBUG("port %s, mask %#hhx, val %#hhx %#hhx\n", dev->node->name, dtd->mask, *((char*)buf), v);

	return 1;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	uint8_t v;
	dt_data_t *dtd;
	gpio_regs_t *regs;


	dtd = ((dev_data_t*)dev->data)->dtd;
	regs = dtd->regs;

	/* write port */
	v = regs->pin & ~dtd->mask;
	v |= (*((char*)buf) << rightmost_bit(dtd->mask)) & dtd->mask;

	regs->port = v;

	DEBUG("port %s, mask %#hhx, val %#hhx\n", dev->node->name, dtd->mask, v);

	return 1;
}

static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	dev_data_t *port;
	sig_tgt_t *tgt;
	signal_t sig;


	port = (dev_data_t*)dev->data;

	switch(request){
	case IOCTL_CFGWR:
		sig = *((signal_t*)data);

		if(sig < SIG_USR0 || sig > SIG_USR3)
			return_errno(E_INVAL);

		tgt = kmalloc(sizeof(sig_tgt_t));

		if(tgt == 0x0)
			return -errno;

		tgt->signal = sig;
		tgt->thread = (thread_t*)sched_running();
		tgt->fd = fd;

		mutex_lock(&port->mtx);
		list_add_tail(port->sig_tgt_lst, tgt);
		mutex_unlock(&port->mtx);

		DEBUG("add %s.%d to signal list for \"%s\"\n", tgt->thread->parent->name, tgt->thread->tid, dev->node->name);

		return E_OK;

	default:
		return_errno(E_NOSUP);
	}
}

static void int_hdlr(int_num_t num, void *data){
	devfs_dev_t *dev;
	dev_data_t *port;
	sig_tgt_t *tgt;


	dev = (devfs_dev_t*)data;
	port = (dev_data_t*)dev->data;

	DEBUG("int %d on \"%s\"\n", num, dev->node->name);

	mutex_lock(&port->mtx);

	list_for_each(port->sig_tgt_lst, tgt)
		usignal_send(tgt->thread, tgt->signal);

	mutex_unlock(&port->mtx);
}

static uint8_t rightmost_bit(uint8_t v){
	uint8_t i;


	i = 0;

	while(v && !(v & 0x1)){
		v >>= 1;
		i++;
	}

	return i;
}
