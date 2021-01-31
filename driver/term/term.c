/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <config/config.h>
#include <kernel/interrupt.h>
#include <kernel/init.h>
#include <kernel/fs.h>
#include <kernel/devfs.h>
#include <kernel/memory.h>
#include <kernel/ksignal.h>
#include <kernel/driver.h>
#include <kernel/inttask.h>
#include <kernel/critsec.h>
#include <driver/term.h>
#include <driver/klog.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>
#include <sys/list.h>


/* types */
typedef struct{
	char const *s;
	size_t len;
} tx_data_t;

typedef struct{
	void *cfg;
	term_itf_t *hw;

	ringbuf_t rx_buf;
	ksignal_t *rx_rdy;
	term_err_t rx_err;

	itask_queue_t tx_queue;
	critsec_lock_t lock;
} term_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);

static size_t puts_noint(char const *s, size_t n, void *term);
static size_t puts_int(char const *s, size_t n, void *term);

static void rx_hdlr(int_num_t num, void *term);
static void tx_hdlr(int_num_t num, void *term);


/* local functions */
static int probe(char const *name, void *dt_data, term_itf_t *itf, term_t **_term){
	void *buf;
	devfs_dev_t *dev;
	devfs_ops_t dev_ops;
	term_t *term;
	f_mode_t fmode_mask;


	fmode_mask = 0;

	if(itf->configure == 0x0 || itf->gets == 0x0 || itf->puts == 0x0)
		goto_errno(err_0, E_INVAL);

	/* allocate terminal */
	term = kmalloc(sizeof(term_t));

	if(term == 0x0)
		goto err_0;

	/* allocate recv buffer */
	buf = 0x0;

	if(itf->rx_int){
		buf = kmalloc(CONFIG_TERM_RXBUF_SIZE);

		if(buf == 0x0)
			goto err_1;
	}

	/* register device */
	if(itf->rx_int == 0){
		fmode_mask = O_NONBLOCK;	// if rx interrupts are not used the device must
									// not be used in blocking mode, therefor the file
									// system default setting (non-blocking) must not
									// be overwritten
	}

	dev_ops.open = 0x0;
	dev_ops.close = 0x0;
	dev_ops.read = read;
	dev_ops.write = write;
	dev_ops.ioctl = ioctl;
	dev_ops.fcntl = 0x0;

	dev = devfs_dev_register(name, &dev_ops, fmode_mask, term);

	if(dev == 0x0)
		goto err_2;

	/* register interrupt */
	if(itf->rx_int && int_register(itf->rx_int, rx_hdlr, term) != 0)
		goto err_3;

	if(itf->tx_int && int_register(itf->tx_int, tx_hdlr, term) != 0)
		goto err_4;

	/* init term */
	term->cfg = dt_data;
	term->hw = itf;
	term->rx_rdy = &dev->node->datain_sig;
	term->rx_err = TERM_ERR_NONE;

	critsec_init(&term->lock);
	itask_queue_init(&term->tx_queue);
	ringbuf_init(&term->rx_buf, buf, CONFIG_TERM_RXBUF_SIZE);

	if(term->hw->configure(term->cfg, term->hw->data) != 0)
		goto err_5;

	if(_term)
		*_term = term;

	return E_OK;


err_5:
	if(itf->tx_int)
		int_release(itf->tx_int);

err_4:
	if(itf->rx_int)
		int_release(itf->rx_int);

err_3:
	devfs_dev_release(dev);

err_2:
	kfree(buf);

err_1:
	kfree(term);

err_0:
	return -errno;
}

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

	itf->puts = puts_noint;
	itf->data = term;

	return itf;
}

interface_probe("terminal", probe_itf);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	size_t len;
	term_t *term;


	term = (term_t*)dev->data;

	/* read */
	if(term->rx_err){
		errno = E_IO;
		return 0;
	}

	critsec_lock(&term->lock);

	if(term->hw->rx_int)	len = ringbuf_read(&term->rx_buf, buf, n);
	else					len = term->hw->gets(buf, n, &term->rx_err, term->hw->data);

	critsec_unlock(&term->lock);

	/* handle terminal flags */
	// handle TERM_FLAG_ECHO
	if(len && (term->hw->get_flags(term->cfg) & TERM_FLAG_ECHO))
		len = write(dev, fd, buf, len);

	return len;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term;


	term = dev->data;

	if(n <= 1 || term->hw->tx_int == 0)
		return puts_noint(buf, n, term);
	return puts_int(buf, n, term);
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
		critsec_lock(&term->lock);
		r = term->hw->configure(data, term->hw->data);
		critsec_unlock(&term->lock);

		if(r != E_OK)
			return -errno;

		memcpy(term->cfg, data, term->hw->cfg_size);
		return E_OK;

	case IOCTL_STATUS:
		memcpy(data, &term->rx_err, sizeof(term_err_t));

		// reset error flags
		term->rx_err = TERM_ERR_NONE;
		return E_OK;

	default:
		return_errno(E_NOSUP);
	}
}

static size_t puts_noint(char const *s, size_t n, void *_term){
	size_t r;
	term_t *term;


	term = (term_t*)_term;

	critsec_lock(&term->lock);
	r = term->hw->puts(s, n, term->hw->data);
	critsec_unlock(&term->lock);

	return r;
}

static size_t puts_int(char const *s, size_t n, void *_term){
	term_t *term;
	tx_data_t data;


	term = (term_t*)_term;

	data.s = s;
	data.len = n;

	(void)itask_issue(&term->tx_queue, &data, term->hw->tx_int);

	return n - data.len;
}

static void rx_hdlr(int_num_t num, void *_term){
	char buf[16];
	size_t len;
	term_t *term;


	term = (term_t*)_term;

	critsec_lock(&term->lock);

	len = term->hw->gets(buf, 16, &term->rx_err, term->hw->data);

	if(ringbuf_write(&term->rx_buf, buf, len) != len)
		term->rx_err |= TERM_ERR_RX_FULL;

	critsec_unlock(&term->lock);

	ksignal_send(term->rx_rdy);
}

static void tx_hdlr(int_num_t num, void *_term){
	term_t *term;
	tx_data_t *data;


	term = (term_t*)_term;

	data = itask_query_data(&term->tx_queue);

	if(data == 0x0)
		return;

	/* output character */
	critsec_lock(&term->lock);
	while(term->hw->putc(*data->s, term->hw->data) != *data->s);
	critsec_unlock(&term->lock);

	data->s++;
	data->len--;

	if(data->len == 0)
		itask_complete(&term->tx_queue, E_OK);
}
