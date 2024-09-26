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
#include <sys/errno.h>
#include <sys/gpio.h>
#include <sys/ioctl.h>
#include <sys/list.h>
#include <sys/mutex.h>
#include <sys/signal.h>
#include <sys/stream.h>
#include <sys/string.h>
#include <sys/types.h>
#include <sys/vector.h>


/* local/static prototypes */
static void int_hdlr(int_num_t num, void *payload);


/* global functions */
intgpio_t gpio_read(gpio_t *gpio, intgpio_t mask){
	gpio_itf_t *itf = gpio->itf;
	gpio_cfg_t *cfg = gpio->cfg;
	intgpio_t v;


	v = itf->read(itf->hw);
	DEBUG("read raw=%#x, masked=%#x\n", v, ((v ^ cfg->invert_mask) & mask) & cfg->in_mask);

	return ((v ^ cfg->invert_mask) & mask) & cfg->in_mask;
}

int gpio_write(gpio_t *gpio, intgpio_t v, intgpio_t mask){
	gpio_itf_t *itf = gpio->itf;
	gpio_cfg_t *cfg = gpio->cfg;

	v = ((itf->read(itf->hw) & ~mask) | ((v ^ cfg->invert_mask) & mask)) & cfg->out_mask;
	DEBUG("write %#x\n", v);

	return itf->write(v, itf->hw);
}

int gpio_int_register(gpio_t *gpio, int_num_t num, intgpio_t mask){
	gpio_siglst_t *sig;


	sig = kcalloc(1, sizeof(gpio_siglst_t));

	if(sig == 0x0)
		return -errno;

	sig->int_num = num;
	sig->mask = mask;

	list_add_tail_safe(gpio->sigs, sig, &gpio->mtx);

	return 0;
}

int gpio_sig_register(gpio_t *gpio, fs_filed_t *fd, gpio_sig_cfg_t *cfg){
	gpio_siglst_t *sig,
				  *queued;


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
	sig->int_num = 0;
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

int gpio_sig_release(gpio_t *gpio, fs_filed_t *fd){
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

void gpio_sig_probe(gpio_t *gpio, fs_filed_t *fd, gpio_sig_cfg_t *cfg){
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
static void *probe(char const *name, void *dt_data, void *dt_itf){
	gpio_cfg_t *dtd = (gpio_cfg_t*)dt_data;
	gpio_itf_t *dti = (gpio_itf_t*)dt_itf;
	gpio_t *gpio;


	gpio = kcalloc(1, sizeof(gpio_t));

	if(gpio == 0x0)
		goto err_0;

	gpio->itf = dti;
	gpio->cfg = dtd;
	gpio->sigs = 0x0;

	mutex_init(&gpio->mtx, MTX_NOINT);

	if(dtd->int_num && int_register(dtd->int_num, int_hdlr, gpio) != 0)
		goto err_1;

	if(dti->configure(dtd, dti->hw) != 0)
		goto err_1;

	gpio->int_state = gpio_read(gpio, dtd->out_mask);

	return gpio;


err_1:
	kfree(gpio);

err_0:
	return 0x0;
}

driver_probe("gpio", probe);

static void int_hdlr(int_num_t num, void *payload){
	gpio_t *gpio = (gpio_t*)payload;
	gpio_itf_t *itf = gpio->itf;
	gpio_cfg_t *cfg = gpio->cfg;
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

		if(sig->int_num == 0)	usignal_send(sig->thread, sig->signum);
		else					int_foretell(sig->int_num);
	}

	mutex_unlock(&gpio->mtx);
}
