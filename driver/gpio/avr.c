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
	// port registers
	struct{
		uint8_t volatile pin,
						 ddr,
						 port;
	} *dev;

	// interrupt registers
	uint8_t volatile *pcicr,
					 *pcmsk;

	uint8_t const pcint_num;

	// configuration
	uint8_t const dir,		// direction, cf. port_dir_t
				  mask;		// pin bits
} dt_data_t;

typedef struct{
	dt_data_t *regs;
	sig_tgt_t *sig_tgt_lst;
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
	dt_data_t *regs;
	devfs_ops_t ops;
	devfs_dev_t *dev;
	dev_data_t *dev_data;


	regs = (dt_data_t*)dt_data;

	/* register device */
	ops.open = 0x0;
	ops.close = close;
	ops.read = regs->dir & PORT_IN ? read : 0x0;
	ops.write = regs->dir & PORT_OUT ? write : 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;

	dev_data = kmalloc(sizeof(dev_data_t));

	if(dev_data == 0x0)
		goto err_0;

	dev_data->regs = dt_data;
	dev_data->sig_tgt_lst = 0x0;

	dev = devfs_dev_register(name, &ops, 0x0, dev_data);

	if(dev == 0x0)
		goto err_1;

	/* configure port */
	regs->dev->port |= regs->mask;

	if(regs->dir & PORT_OUT)
		regs->dev->ddr |= regs->mask;

	/* configure interrupt */
	if(regs->pcicr != 0x0 && regs->pcmsk != 0x0){
		*regs->pcicr |= (0x1 << (regs->pcint_num / 8));
		*regs->pcmsk |= regs->mask;

		if(int_register(INT_PCINT0 + regs->pcint_num / 8, int_hdlr, dev))
			goto err_2;
	}

	return E_OK;


err_2:
	devfs_dev_release(dev);

err_1:
	kfree(dev_data);

err_0:
	return -errno;
}

device_probe("avr,gpio", probe);

static int close(struct devfs_dev_t *dev, fs_filed_t *fd){
	dev_data_t *dev_data;
	sig_tgt_t *tgt;


	dev_data = (dev_data_t*)dev->data;

	list_for_each(dev_data->sig_tgt_lst, tgt){
		if(tgt->fd != fd)
			continue;

		DEBUG("remove %s.%d from signal list for \"%s\"\n", tgt->thread->parent->name, tgt->thread->tid, dev->node->name);

		list_rm(dev_data->sig_tgt_lst, tgt);
		kfree(tgt);
	}

	return E_OK;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dt_data_t *regs;
	uint8_t v;


	regs = ((dev_data_t*)dev->data)->regs;

	/* read port */
	v = regs->dev->pin;
	*((char*)buf) = (v & regs->mask) >> rightmost_bit(regs->mask);

	DEBUG("port %s, mask %#hhx, val %#hhx %#hhx\n", dev->node->name, regs->mask, *((char*)buf), v);

	return 1;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	uint8_t v;
	dt_data_t *regs;


	regs = ((dev_data_t*)dev->data)->regs;

	/* write port */
	v = regs->dev->pin & ~regs->mask;
	v |= (*((char*)buf) << rightmost_bit(regs->mask)) & regs->mask;

	regs->dev->port = v;

	DEBUG("port %s, mask %#hhx, val %#hhx\n", dev->node->name, regs->mask, v);

	return 1;
}

static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	dev_data_t *dev_data;
	sig_tgt_t *tgt;
	signal_t sig;


	dev_data = (dev_data_t*)dev->data;

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

		list_add_tail(dev_data->sig_tgt_lst, tgt);

		DEBUG("add %s.%d to signal list for \"%s\"\n", tgt->thread->parent->name, tgt->thread->tid, dev->node->name);

		return E_OK;

	default:
		return_errno(E_NOSUP);
	}
}

static void int_hdlr(int_num_t num, void *data){
	devfs_dev_t *dev;
	dev_data_t *dev_data;
	sig_tgt_t *tgt;


	dev = (devfs_dev_t*)data;
	dev_data = (dev_data_t*)dev->data;

	DEBUG("int %d on \"%s\"\n", num, dev->node->name);

	list_for_each(dev_data->sig_tgt_lst, tgt)
		usignal_send(tgt->thread, tgt->signal);
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
