/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <kernel/driver.h>
#include <kernel/fs.h>
#include <kernel/devfs.h>
#include <driver/term.h>
#include <driver/klog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/string.h>


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);

static size_t klog_puts(char const *s, size_t n, void *hw);
static size_t flputs(char const *s, size_t n, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_cfg_t *dtd = (term_cfg_t*)dt_data;
	term_itf_t *dti = (term_itf_t*)dt_itf;
	devfs_dev_t *dev;
	devfs_ops_t dev_ops;
	term_t *term;
	klog_itf_t *klog;


	/* register device */
	dev_ops.open = 0x0;
	dev_ops.close = 0x0;
	dev_ops.read = read;
	dev_ops.write = write;
	dev_ops.ioctl = ioctl;
	dev_ops.fcntl = 0x0;
	dev_ops.mmap = 0x0;

	dev = devfs_dev_register(name, &dev_ops, 0x0);

	if(dev == 0x0)
		goto err_0;

	/* create terminal */
	term = term_create(dti, dtd, dev->node);

	if(term == 0x0)
		goto err_1;

	dev->payload = term;

	/* register interrupt */
	if(dti->rx_int && int_register(dti->rx_int, term_rx_hdlr, term) != 0)
		goto err_2;

	if(dti->tx_int && int_register(dti->tx_int, term_tx_hdlr, term) != 0)
		goto err_3;

	if(dti->rx_int)
		term->node->timeout_us = 0;

	/* init term */
	if(dti->configure != 0x0 && dti->configure(term->cfg, dti->cfg, dti->hw) != 0)
		goto err_4;

	/* create terminal interface */
	klog = kmalloc(sizeof(klog_itf_t));

	if(klog == 0x0)
		goto err_4;

	klog->puts = klog_puts;
	klog->hw = term;

	return klog;


err_4:
	if(dti->tx_int)
		int_release(dti->tx_int);

err_3:
	if(dti->rx_int)
		int_release(dti->rx_int);

err_2:
	term_destroy(term);

err_1:
	devfs_dev_release(dev);

err_0:
	return 0x0;
}

driver_probe("terminal", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term = (term_t*)dev->payload;


	/* read */
	n = term_gets(term, buf, n);

	if(term->errno)
		goto_errno(err, term->errno);

	/* handle terminal flags */
	if(term_flags_apply(term, buf, n, 1, TFT_I,  term->cfg->iflags) == 0x0)
		goto_errno(err, E_IO);

	if(term_flags_apply(term, buf, n, n, TFT_L, term->cfg->lflags) == 0x0)
		goto_errno(err, E_IO);

	return n;


err:
	term->errno = 0;

	return 0;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term = (term_t*)dev->payload;


	if(flputs(buf, n, term) == 0 && n != 0)
		goto_errno(err, term->errno);

	return n;


err:
	term->errno = 0;

	return 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	term_t *term = (term_t*)dev->payload;


	if(n != sizeof(term_cfg_t) && n != (sizeof(term_cfg_t) + term->itf->cfg_size))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(arg, term->cfg, sizeof(term_cfg_t));

		if(n > sizeof(term_cfg_t))
			memcpy(arg + sizeof(term_cfg_t), term->itf->cfg, term->itf->cfg_size);

		return 0;

	case IOCTL_CFGWR:
		if(n > sizeof(term_cfg_t)){
			if(term->itf->configure == 0x0)
				return_errno(E_NOSUP);

			if(term->itf->configure(arg, arg + sizeof(term_cfg_t), term->itf->hw) != 0)
				return -errno;

			memcpy(term->itf->cfg, arg + sizeof(term_cfg_t), term->itf->cfg_size);
		}

		memcpy(term->cfg, arg, sizeof(term_cfg_t));

		return 0;

	default:
		return_errno(E_NOSUP);
	}
}

static size_t flputs(char const *_s, size_t n, void *hw){
	term_t *term = (term_t*)hw;
	size_t n_put;
	char s[n];


	memcpy(s, _s, n);

	/* handle terminal iflags */
	_s = term_flags_apply(term, s, n, 1, TFT_O, term->cfg->oflags);
	n_put = _s - s;

	if(_s == 0x0)
		return 0;

	/* perform write */
	return (term_puts(term, _s, n - n_put) + n_put == n) ? n : 0;
}

static size_t klog_puts(char const *s, size_t n, void *hw){
	term_t *term = (term_t*)hw;
	size_t r;


	mutex_lock(&term->node->mtx);
	r = flputs(s, n, hw);
	mutex_unlock(&term->node->mtx);

	return r;
}
