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


/* local/static prototypes */
static void int_hdlr(int_num_t num, void *payload);


/* global functions */
gpio_t *gpio_create(gpio_itf_t *itf, gpio_cfg_t *cfg){
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

	gpio->itf = itf;
	gpio->cfg = cfg;
	gpio->sigs = 0x0;
	gpio->int_state = 0;

	mutex_init(&gpio->mtx, MTX_NOINT);

	return gpio;


err:
	return 0x0;
}

void gpio_destroy(gpio_t *gpio){
	kfree(gpio);
}

int gpio_configure(gpio_t *gpio){
	gpio_itf_t *itf = gpio->itf;
	gpio_cfg_t *cfg = gpio->cfg;


	if(itf->configure(cfg, itf->hw) != 0)
		return -errno;

	gpio->int_state = gpio_read(gpio);

	if(cfg->int_num && int_register(cfg->int_num, int_hdlr, gpio) != 0)
		return -errno;

	return 0;
}

intgpio_t gpio_read(gpio_t *gpio){
	gpio_itf_t *itf = gpio->itf;
	gpio_cfg_t *cfg = gpio->cfg;


	DEBUG("read raw=%#x, masked=%#x\n", itf->read(itf->hw), (itf->read(itf->hw) ^ cfg->invert_mask) & cfg->in_mask);

	return ((itf->read(itf->hw) ^ cfg->invert_mask) & cfg->in_mask);
}

int gpio_write(gpio_t *gpio, intgpio_t v){
	gpio_itf_t *itf = gpio->itf;
	gpio_cfg_t *cfg = gpio->cfg;


	v = (itf->read(itf->hw) & ~cfg->out_mask) | ((v ^ cfg->invert_mask) & cfg->out_mask);
	DEBUG("write %#x\n", v);

	return itf->write(v, itf->hw);
}

int gpio_int_register(gpio_t *gpio, fs_filed_t *fd, gpio_int_cfg_t *cfg){
	gpio_siglst_t *sig,
				  *queued;


	if(cfg->mode == GM_STRICT && (pin_mask & ~cfg->int_mask))
		return_errno(E_INVAL);

	if(cfg->signum < SIG_USR0 || cfg->signum > SIG_USR3)
		return_errno(E_INVAL);

	cfg->mask &= gpio->cfg->int_mask;

	queued = list_find_safe(gpio->sigs, fd, fd, &gpio->mtx);
	sig = queued ? queued : kmalloc(sizeof(gpio_siglst_t));

	if(sig == 0x0)
		return -errno;

	sig->signum = cfg->signum;
	sig->thread = sched_running();
	sig->fd = fd;
	sig->mask = cfg->mask;

	if(queued == 0x0)
		list_add_tail_safe(gpio->sigs, sig, &gpio->mtx);

	DEBUG("%s %s.%d to %s's signal list, mask = %#x\n",
		queued ? "update" : "register",
		sig->thread->parent->name,
		sig->thread->tid,
		fd->node->name,
		cfg->mask
	);

	return 0;
}

int gpio_int_release(gpio_t *gpio, fs_filed_t *fd){
	gpio_siglst_t *sig;


	sig = list_find_safe(gpio->sigs, fd, fd, &gpio->mtx);

	if(sig == 0x0)
		return_errno(E_UNAVAIL);

	list_rm_safe(gpio->sigs, sig, &gpio->mtx);
	kfree(sig);

	DEBUG("remove %s.%d from %s's signal list\n",
		sig->thread->parent->name,
		sig->thread->tid,
		fd->node->name
	);

	return 0;
}

void gpio_int_probe(gpio_t *gpio, fs_filed_t *fd, gpio_int_cfg_t *cfg){
	gpio_siglst_t *sig;


	sig = list_find_safe(gpio->sigs, fd, fd, &gpio->mtx);

	if(sig == 0x0){
		cfg->signum = 0;
		cfg->mask = 0;

		return;
	}

	cfg->signum = sig->signum;
	cfg->mask = sig->mask;
}


/* local functions */
static void int_hdlr(int_num_t num, void *payload){
	gpio_t *gpio = (gpio_t*)payload;
	gpio_itf_t *itf = gpio->itf;
	gpio_cfg_t * cfg = gpio->cfg;
	gpio_siglst_t *sig;
	intgpio_t v,
			  changed;


	v = ((itf->read(itf->hw) ^ cfg->invert_mask) & cfg->int_mask);
	changed = v ^ gpio->int_state;
	gpio->int_state = v;

	mutex_lock(&gpio->mtx);

	list_for_each(gpio->sigs, sig){
		if((changed & sig->mask) == 0)
			continue;

		usignal_send(sig->thread, sig->signum);
	}

	mutex_unlock(&gpio->mtx);
}
