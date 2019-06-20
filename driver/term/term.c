/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <config/config.h>
#include <kernel/init.h>
#include <kernel/devfs.h>
#include <kernel/memory.h>
#include <kernel/ksignal.h>
#include <kernel/driver.h>
#include <kernel/opt.h>
#include <driver/term.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>
#include <sys/mutex.h>
#include <sys/list.h>


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);
static size_t puts(char const *s, size_t n, term_t *term) __attribute__((noinline));
static void rx_hdlr(int_num_t num, void *data);
static void tx_hdlr(int_num_t num, void *data);


/* local functions */
int probe(char const *name, void *dt_data, void *dt_itf){
	void *buf;
	devfs_dev_t *dev;
	devfs_ops_t dev_ops;
	term_t *term;
	term_itf_t *itf;
	f_mode_t fmode_mask;


	fmode_mask = 0;
	itf = (term_itf_t*)dt_itf;

	if(itf->configure == 0x0 || itf->gets == 0x0 || itf->puts == 0x0)
		goto_errno(err_0, E_INVAL);

	/* allocate terminal */
	term = kmalloc(sizeof(term_t));

	if(term == 0x0)
		goto_errno(err_0, E_NOMEM);

	/* allocate recv buffer */
	buf = 0x0;

	if(itf->rx_int){
		buf = kmalloc(CONFIG_TERM_RXBUF_SIZE);

		if(buf == 0x0)
			goto_errno(err_1, E_NOMEM);
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
	term->cfg = kopt.term_cfg;
	term->hw = dt_itf;
	term->rx_rdy = &dev->node->rd_sig;
	term->rx_err = TE_NONE;
	term->tx_queue_head = 0x0;
	term->tx_queue_tail = 0x0;

	mutex_init(&term->tx_mtx, 0);
	ringbuf_init(&term->rx_buf, buf, CONFIG_TERM_RXBUF_SIZE);

	if(term->hw->configure(&term->cfg, term->hw->regs) != 0)
		goto err_5;

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

device_probe("terminal", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	int len;
	term_t *term;


	term = (term_t*)dev->data;

	/* read */
	if(term->rx_err){
		errno = E_IO;
		return 0;
	}

	if(term->rx_buf.data == 0x0)
		len = term->hw->gets(buf, n, &term->rx_err, term->hw->regs);
	else
		len = ringbuf_read(&term->rx_buf, buf, n);

	/* handle terminal flags */
	// handle TF_ECHO
	if(len > 0 && (term->cfg.flags & TF_ECHO)){
		if(term->hw->puts(buf, n, term->hw->regs) != E_OK)
			return 0;
	}

	return len;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return puts(buf, n, dev->data);
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	term_t *term;


	term = (term_t*)dev->data;

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, &term->cfg, sizeof(term_cfg_t));
		return E_OK;

	case IOCTL_CFGWR:
		if(term->hw->configure((term_cfg_t*)data, term->hw->regs) != E_OK)
			return -errno;

		term->cfg = *((term_cfg_t*)data);
		return E_OK;

	case IOCTL_STATUS:
		memcpy(data, &term->rx_err, sizeof(term_err_t));

		// reset error flags
		term->rx_err = TE_NONE;
		return E_OK;

	default:
		return_errno(E_NOIMP);
	}
}

static size_t puts(char const *s, size_t n, term_t *term){
	term_txqueue_t buf;
	bool start;


	if(term->hw->tx_int == 0 || n <= 1)
		return term->hw->puts(s, n, term->hw->regs);

	buf.data = s;
	buf.len = n;
	ksignal_init(&buf.fin);

	mutex_lock(&term->tx_mtx);

	list1_add_tail(term->tx_queue_head, term->tx_queue_tail, &buf);
	start = (list_first(term->tx_queue_head) == &buf);

	mutex_unlock(&term->tx_mtx);

	if(start)
		tx_hdlr(term->hw->tx_int, term);

	ksignal_wait(&buf.fin);

	return n;
}

static void rx_hdlr(int_num_t num, void *data){
	char buf[16];
	size_t len;
	term_t *term;


	term = (term_t*)data;

	len = term->hw->gets(buf, 16, &term->rx_err, term->hw->regs);

	if(ringbuf_write(&term->rx_buf, buf, len) != len)
		term->rx_err |= TE_RX_FULL;

	ksignal_send(term->rx_rdy);
}

static void tx_hdlr(int_num_t num, void *data){
	term_t *term;
	term_txqueue_t *buf;


	term = (term_t*)data;

	mutex_lock(&term->tx_mtx);

	buf = list_first(term->tx_queue_head);

	if(buf && buf->len == 0){
		list1_rm_head(term->tx_queue_head, term->tx_queue_tail);
		ksignal_send(&buf->fin);
	}

	buf = list_first(term->tx_queue_head);

	mutex_unlock(&term->tx_mtx);

	if(buf && buf->len){
		term->hw->putc(*buf->data, term->hw->regs);
		buf->data++;
		buf->len--;
	}
}
