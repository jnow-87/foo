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


/* local/static prototypes */
static int close(devfs_dev_t *dev, fs_filed_t *fd);
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);

static void int_hdlr(int_num_t num, void *gpio);

static int int_set(gpio_t *gpio, gpio_int_cfg_t *cfg, fs_filed_t *fd);
static int int_clear(gpio_t *gpio, gpio_siglst_t *sig);


/* global functions */
gpio_t *gpio_create(char const *name, gpio_ops_t *ops, gpio_cfg_t *cfg, void *hw){
	gpio_t *gpio;
	devfs_ops_t dev_ops;
	devfs_dev_t *dev;


	/* sanitise configuration */
	if(cfg->mode == GM_STRICT){
		if((cfg->out_mask & ~cfg->pin_mask)
		|| (cfg->in_mask & ~cfg->pin_mask)
		|| (cfg->int_mask & ~cfg->pin_mask)
		|| (cfg->invert_mask & ~cfg->pin_mask)
		)
			goto_errno(err_0, E_INVAL);
	}

	cfg->in_mask &= cfg->pin_mask;
	cfg->out_mask &= cfg->pin_mask;
	cfg->int_mask &= cfg->pin_mask;
	cfg->invert_mask &= cfg->pin_mask;

	/* allocated gpio struct */
	gpio = kmalloc(sizeof(gpio_t));

	if(gpio == 0x0)
		goto err_0;

	gpio->ops = *ops;
	gpio->cfg = cfg;
	gpio->hw = hw;
	gpio->sigs = 0x0;
	gpio->dev = 0x0;

	mutex_init(&gpio->mtx, MTX_NOINT);

	if(ops->configure(cfg, hw) != 0)
		goto err_1;

	/* register devfs device */
	dev_ops.open = 0x0;
	dev_ops.close = close;
	dev_ops.read = cfg->in_mask ? read : 0x0;
	dev_ops.write = cfg->out_mask ? write : 0x0;
	dev_ops.ioctl = ioctl;
	dev_ops.fcntl = 0x0;

	dev = devfs_dev_register(name, &dev_ops, gpio);

	if(dev == 0x0)
		goto err_1;

	gpio->dev = dev;

	/* register interrupt */
	if(cfg->int_num && int_register(cfg->int_num, int_hdlr, gpio) != 0)
		goto err_1;

	return gpio;


err_1:
	gpio_destroy(gpio);

err_0:
	return 0x0;
}

void gpio_destroy(gpio_t *gpio){
	gpio_siglst_t *sig;


	int_release(gpio->cfg->int_num);

	list_for_each(gpio->sigs, sig)
		int_clear(gpio, sig);

	if(gpio->dev != 0x0)
		devfs_dev_release(gpio->dev);

	kfree(gpio);
}


/* local functions */
static int close(devfs_dev_t *dev, fs_filed_t *fd){
	gpio_t *gpio;
	gpio_siglst_t *sig;


	gpio = (gpio_t*)dev->data;
	sig = list_find_safe(gpio->sigs, fd, fd, &gpio->mtx);

	return (sig != 0x0) ? int_clear(gpio, sig) : E_OK;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	gpio_t *gpio;


	if(n != sizeof(gpio_type_t))
		goto_errno(err, E_LIMIT);

	gpio = (gpio_t*)dev->data;
	*((gpio_type_t*)buf) = ((gpio->ops.read(gpio->hw) ^ gpio->cfg->invert_mask) & gpio->cfg->in_mask);

	DEBUG("%s read %#x\n", gpio->dev->node->name, *((gpio_type_t*)buf));

	return n;


err:
	return 0;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	gpio_t *gpio;
	gpio_type_t c,
				v;


	if(n != sizeof(gpio_type_t))
		goto_errno(err, E_LIMIT);

	gpio = (gpio_t*)dev->data;
	v = *((gpio_type_t*)buf);

	if(gpio->cfg->mode == GM_STRICT && (v & ~gpio->cfg->out_mask))
		goto_errno(err, E_INVAL);

	c = gpio->ops.read(gpio->hw) & ~gpio->cfg->out_mask;
	v = c | ((v ^ gpio->cfg->invert_mask) & gpio->cfg->out_mask);

	DEBUG("%s write %#x\n", gpio->dev->node->name, v);

	return (gpio->ops.write(v, gpio->hw) != 0) ? 0 : n;


err:
	return 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	switch(request){
	case IOCTL_CFGWR:	return int_set(dev->data, data, fd);
	default:			return_errno(E_NOSUP);
	}
}

static void int_hdlr(int_num_t num, void *data){
	gpio_t *gpio;
	gpio_siglst_t *sig;
	gpio_type_t v;


	gpio = (gpio_t*)data;
	v = gpio->ops.read(gpio->hw);

	DEBUG("%s interrupt with %#x\n", gpio->dev->node->name, v);

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
		return int_clear(gpio, queued);

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
		gpio->dev->node->name,
		cfg->mask
	);

	if(queued == 0x0)
		list_add_tail_safe(gpio->sigs, sig, &gpio->mtx);

	return E_OK;
}

static int int_clear(gpio_t *gpio, gpio_siglst_t *sig){
	DEBUG("remove %s.%d from %s's signal list\n",
		sig->thread->parent->name,
		sig->thread->tid,
		gpio->dev->node->name
	);

	list_rm_safe(gpio->sigs, sig, &gpio->mtx);
	kfree(sig);

	return E_OK;
}
