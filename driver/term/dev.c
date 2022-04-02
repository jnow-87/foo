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
static int probe(char const *name, void *dt_data, term_itf_t *hw, term_t **_term);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);

static size_t flputs(char const *s, size_t n, void *term);


/* local functions */
static int probe_dev(char const *name, void *dt_data, void *dt_itf){
	return probe(name, dt_data, dt_itf, 0x0);
}

device_probe("terminal", probe_dev);

static void *probe_itf(char const *name, void *dt_data, void *dt_itf){
	term_t *term;
	klog_itf_t *itf;


	if(probe(name, dt_data, dt_itf, &term))
		return 0x0;

	itf = kmalloc(sizeof(klog_itf_t));

	if(itf == 0x0)
		return 0x0;

	itf->puts = flputs;
	itf->data = term;

	return itf;
}

interface_probe("terminal", probe_itf);

static int probe(char const *name, void *dt_data, term_itf_t *hw, term_t **_term){
	devfs_dev_t *dev;
	devfs_ops_t dev_ops;
	term_t *term;


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

	/* init term */
	if(term->hw->configure(term->cfg, term->hw->data) != 0)
		goto err_4;

	if(_term)
		*_term = term;

	return E_OK;


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
	return -errno;
}

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term;
	term_flags_t *flags;


	term = (term_t*)dev->data;

	/* read */
	if(term->rx_err)
		goto_errno(err, E_IO);

	n = term_gets(term, buf, n);

	/* handle terminal flags */
	flags = term_flags(term);

	if(term_flags_apply(term, buf, n, 1, TFT_I, flags->iflags) == 0x0)
		goto_errno(err, E_IO);

	if(term_flags_apply(term, buf, n, n, TFT_L, flags->lflags) == 0x0)
		goto_errno(err, E_IO);

	return n;


err:
	return 0;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	if(flputs(buf, n, dev->data) == 0 && n != 0)
		goto_errno(err, E_IO);

	return n;


err:
	return 0;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	int r;
	term_t *term;


	term = (term_t*)dev->data;

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, term->cfg, term->hw->cfg_size);
		return E_OK;

	case IOCTL_CFGWR:
		r = term->hw->configure(data, term->hw->data);

		if(r != E_OK)
			return -errno;

		memcpy(term->cfg, data, term->hw->cfg_size);
		return E_OK;

	case IOCTL_STATUS:
		memcpy(data, &term->rx_err, sizeof(term_err_t));

		// reset error flags
		term->rx_err = TERR_NONE;
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
