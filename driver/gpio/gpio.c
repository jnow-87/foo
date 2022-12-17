/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <kernel/usignal.h>
#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/fs.h>
#include <kernel/kprintf.h>
#include <driver/gpio.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/mutex.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/ioctl.h>
#include <sys/gpio.h>
#include <sys/string.h>
#include <sys/stream.h>


/* types */
typedef struct{
	uint8_t nports;
} dt_data_t;


/* local/static prototypes */
static devfs_dev_t *dev_create(char const *name, gpio_t *gpio);
static void dev_destroy(devfs_dev_t *dev, gpio_t *gpio);

static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);

static void int_hdlr(int_num_t num, void *payload);

static int int_set(gpio_t *gpio, gpio_int_cfg_t *cfg, fs_filed_t *fd);
static int int_clear(gpio_t *gpio, gpio_siglst_t *sig, fs_filed_t *fd);


/* global functions */
gpio_t *gpio_create(gpio_ops_t *ops, gpio_cfg_t *cfg, void *hw){
	gpio_t *gpio;


	/* sanitise configuration */
	if(cfg->mode == GM_STRICT){
		if((cfg->out_mask & ~cfg->pin_mask)
		|| (cfg->in_mask & ~cfg->pin_mask)
		|| (cfg->int_mask & ~cfg->pin_mask)
		|| (cfg->invert_mask & ~cfg->pin_mask)
		)
			goto_errno(err, E_INVAL);
	}

	cfg->in_mask &= cfg->pin_mask;
	cfg->out_mask &= cfg->pin_mask;
	cfg->int_mask &= cfg->pin_mask;
	cfg->invert_mask &= cfg->pin_mask;

	/* allocated gpio struct */
	gpio = kmalloc(sizeof(gpio_t));

	if(gpio == 0x0)
		goto err;

	gpio->ops = *ops;
	gpio->cfg = cfg;
	gpio->hw = hw;
	gpio->sigs = 0x0;

	mutex_init(&gpio->mtx, MTX_NOINT);

	return gpio;


err:
	return 0x0;
}

void gpio_destroy(gpio_t *gpio){
	kfree(gpio);
}

gpio_int_t gpio_read(gpio_t *gpio){
	return ((gpio->ops.read(gpio->hw) ^ gpio->cfg->invert_mask) & gpio->cfg->in_mask);
}

int gpio_write(gpio_t *gpio, gpio_int_t v){
	gpio_int_t c;


	if(gpio->cfg->mode == GM_STRICT && (v & ~gpio->cfg->out_mask))
		return_errno(E_INVAL);

	c = gpio->ops.read(gpio->hw) & ~gpio->cfg->out_mask;
	v = c | ((v ^ gpio->cfg->invert_mask) & gpio->cfg->out_mask);

	return gpio->ops.write(v, gpio->hw);
}


/* local functions */
static void *probe_single(char const *name, void *dt_data, void *dt_itf){
	(void)dev_create(name, dt_itf);

	return 0x0;
}

driver_probe("gpio,dev", probe_single);

static void *probe_multi(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_t **gpios = (gpio_t**)dt_itf;
	size_t 	name_len = strlen(name);
	char port_name[name_len + 4];
	devfs_dev_t *devs[dtd->nports];


	memset(devs, 0x0, sizeof(devfs_dev_t*));

	strcpy(port_name, name);
	port_name[name_len + 1] = 0;

	for(uint8_t i=0; i<dtd->nports; i++){
		snprintf(port_name + name_len, 3, "%u", i);

		devs[i] = dev_create(port_name, gpios[i]);

		if(devs[i] == 0x0)
			goto err;
	}

	return 0x0;


err:
	for(uint8_t i=0; i<dtd->nports && devs[i]!=0x0; i++){
		dev_destroy(devs[i], gpios[i]);
	}

	return 0x0;
}

driver_probe("gpio,multi-dev", probe_multi);

static devfs_dev_t *dev_create(char const *name, gpio_t *gpio){
	gpio_cfg_t *cfg = gpio->cfg;
	devfs_dev_t *dev;
	devfs_ops_t ops;


	/* register devfs device */
	ops.open = 0x0;
	ops.close = close;
	ops.read = cfg->in_mask ? read : 0x0;
	ops.write = cfg->out_mask ? write : 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;
	ops.mmap = 0x0;

	dev = devfs_dev_register(name, &ops, gpio);

	if(dev == 0x0)
		goto err_0;

	/* register interrupt */
	if(cfg->int_num && int_register(cfg->int_num, int_hdlr, dev) != 0)
		goto err_1;

	return dev;


err_1:
	devfs_dev_release(dev);

err_0:
	return 0x0;
}

static void dev_destroy(devfs_dev_t *dev, gpio_t *gpio){
	devfs_dev_release(dev);

	if(gpio->cfg->int_num)
		int_release(gpio->cfg->int_num);
}

static int close(devfs_dev_t *dev, fs_filed_t *fd){
	gpio_t *gpio = (gpio_t*)dev->payload;
	gpio_siglst_t *sig;


	sig = list_find_safe(gpio->sigs, fd, fd, &gpio->mtx);

	return (sig != 0x0) ? int_clear(gpio, sig, fd) : 0;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	gpio_t *gpio = (gpio_t*)dev->payload;
	gpio_int_t *v = (gpio_int_t*)buf;


	if(n != sizeof(gpio_int_t))
		goto_errno(err, E_LIMIT);

	*v = gpio_read(gpio);;

	DEBUG("%s read %#x\n", dev->node->name, *v);

	return n;


err:
	return 0;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	gpio_t *gpio = (gpio_t*)dev->payload;
	gpio_int_t v = *((gpio_int_t*)buf);


	if(n != sizeof(gpio_int_t))
		goto_errno(err, E_LIMIT);

	DEBUG("%s write %#x\n", dev->node->name, v);

	return (gpio_write(gpio, v) != 0) ? 0 : n;


err:
	return 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	if(n != sizeof(gpio_int_cfg_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGWR:	return int_set(dev->payload, arg, fd);
	default:			return_errno(E_NOSUP);
	}
}

static void int_hdlr(int_num_t num, void *payload){
	devfs_dev_t *dev = (devfs_dev_t*)payload;
	gpio_t *gpio = (gpio_t*)dev->payload;
	gpio_siglst_t *sig;
	gpio_int_t v;


	v = gpio->ops.read(gpio->hw);

	DEBUG("%s interrupt with %#x\n", dev->node->name, v);

	mutex_lock(&gpio->mtx);

	list_for_each(gpio->sigs, sig){
		if((v & sig->mask) == 0)
			continue;

		usignal_send(sig->thread, sig->sig);
	}

	mutex_unlock(&gpio->mtx);
}

static int int_set(gpio_t *gpio, gpio_int_cfg_t *cfg, fs_filed_t *fd){
	gpio_siglst_t *sig,
				  *queued;


	if(gpio->cfg->mode == GM_STRICT && (cfg->mask & ~gpio->cfg->int_mask))
		return_errno(E_INVAL);

	if(cfg->sig < SIG_USR0 || cfg->sig > SIG_USR3)
		return_errno(E_INVAL);

	cfg->mask &= gpio->cfg->int_mask;

	queued = list_find_safe(gpio->sigs, fd, fd, &gpio->mtx);

	if(queued != 0x0 && cfg->mask == 0x0)
		return int_clear(gpio, queued, fd);

	sig = (queued == 0x0) ? kmalloc(sizeof(gpio_siglst_t)) : queued;

	if(sig == 0x0)
		return -errno;

	sig->sig = cfg->sig;
	sig->thread = (thread_t*)sched_running();
	sig->fd = fd;
	sig->mask = cfg->mask;

	DEBUG("%s %s.%d to %s's signal list, mask = %#x\n",
		(queued == 0x0) ? "add" : "update",
		sig->thread->parent->name,
		sig->thread->tid,
		fd->node->name,
		cfg->mask
	);

	if(queued == 0x0)
		list_add_tail_safe(gpio->sigs, sig, &gpio->mtx);

	return 0;
}

static int int_clear(gpio_t *gpio, gpio_siglst_t *sig, fs_filed_t *fd){
	DEBUG("remove %s.%d from %s's signal list\n",
		sig->thread->parent->name,
		sig->thread->tid,
		fd->node->name
	);

	list_rm_safe(gpio->sigs, sig, &gpio->mtx);
	kfree(sig);

	return 0;
}
