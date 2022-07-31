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
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data, size_t n);

static size_t flputs(char const *s, size_t n, void *term);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	devfs_dev_t *dev;
	devfs_ops_t dev_ops;
	term_t *term;
	term_itf_t *hw;
	klog_itf_t *itf;


	hw = (term_itf_t*)dt_itf;

	/* register device */
	dev_ops.open = 0x0;
	dev_ops.close = 0x0;
	dev_ops.read = read;
	dev_ops.write = write;
	dev_ops.ioctl = ioctl;
	dev_ops.fcntl = 0x0;

	dev = devfs_dev_register(name, &dev_ops, 0x0);

	if(dev == 0x0)
		goto err_0;

	/* create terminal */
	term = term_create(hw, dt_data, dev->node);

	if(term == 0x0)
		goto err_1;

	dev->data = term;

	/* register interrupt */
	if(hw->rx_int && int_register(hw->rx_int, term_rx_hdlr, term) != 0)
		goto err_2;

	if(hw->tx_int && int_register(hw->tx_int, term_tx_hdlr, term) != 0)
		goto err_3;

	if(hw->rx_int)
		term->node->timeout_us = 0;

	/* init term */
	if(term->hw->configure(term->cfg, term->hw->data) != 0)
		goto err_4;

	/* create terminal interface */
	itf = kmalloc(sizeof(klog_itf_t));

	if(itf == 0x0)
		goto err_4;

	itf->puts = flputs;
	itf->data = term;

	return itf;


err_4:
	if(hw->tx_int)
		int_release(hw->tx_int);

err_3:
	if(hw->rx_int)
		int_release(hw->rx_int);

err_2:
	term_destroy(term);

err_1:
	devfs_dev_release(dev);

err_0:
	return 0x0;
}

driver_probe("terminal", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term;
	term_flags_t *flags;


	term = (term_t*)dev->data;

	/* read */
	n = term_gets(term, buf, n);

	if(term->errno)
		goto_errno(err, term->errno);

	/* handle terminal flags */
	flags = term_flags(term);

	if(term_flags_apply(term, buf, n, 1, TFT_I, flags->iflags) == 0x0)
		goto_errno(err, E_IO);

	if(term_flags_apply(term, buf, n, n, TFT_L, flags->lflags) == 0x0)
		goto_errno(err, E_IO);

	return n;


err:
	term->errno = E_OK;

	return 0;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term;


	term = (term_t*)dev->data;

	if(flputs(buf, n, term) == 0 && n != 0)
		goto_errno(err, term->errno);

	return n;


err:
	term->errno = E_OK;

	return 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data, size_t n){
	term_t *term;


	term = (term_t*)dev->data;

	if(n != sizeof(term_cfg_t) && n != (sizeof(term_cfg_t) + term->hw->cfg_size))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, term->cfg, n);
		return E_OK;

	case IOCTL_CFGWR:
		if(n > sizeof(term_cfg_t)){
			if(term->hw->configure(data, term->hw->data) != 0)
				return -errno;
		}

		memcpy(term->cfg, data, n);
		return E_OK;

	default:
		return_errno(E_NOSUP);
	}
}

static size_t flputs(char const *_s, size_t n, void *_term){
	size_t n_put;
	char s[n];
	term_t *term;
	term_flags_t *flags;


	term = (term_t*)_term;

	memcpy(s, _s, n);

	/* handle terminal iflags */
	flags = term_flags(term);

	_s = term_flags_apply(term, s, n, 1, TFT_O, flags->oflags);
	n_put = _s - s;

	if(_s == 0x0)
		return 0;

	/* perform write */
	return (term_puts(term, _s, n - n_put) + n_put == n) ? n : 0;
}
