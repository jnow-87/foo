/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/kprintf.h>
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
#include <sys/math.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>


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

typedef char *(*flag_hdlr_t)(char *s, size_t idx, size_t n, term_t *term);


/* local/static prototypes */
// syscall
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);

// hardware access
static size_t flputs(char const *s, size_t n, void *term);
static size_t puts(char const *s, size_t n, term_t *term);

static void rx_hdlr(int_num_t num, void *term);
static void tx_hdlr(int_num_t num, void *term);

// line discipline
static char *handle_flags(char *s, size_t n, size_t incr, uint8_t flags, flag_hdlr_t *hdlr, term_t *term);

static char *i_crnl(char *s, size_t idx, size_t n, term_t *term);
static char *i_nlcr(char *s, size_t idx, size_t n, term_t *term);

static char *o_crnl(char *s, size_t idx, size_t n, term_t *term);
static char *o_nlcr(char *s, size_t idx, size_t n, term_t *term);

static char *l_echo(char *s, size_t idx, size_t n, term_t *term);


/* static variables */
static flag_hdlr_t iflag_hdlr[] = {
	i_crnl,
	i_nlcr,
	0x0
};

static flag_hdlr_t oflag_hdlr[] = {
	o_crnl,
	o_nlcr,
	0x0
};

static flag_hdlr_t lflag_hdlr[] = {
	l_echo,
	0x0
};


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
	term->rx_err = TERR_NONE;

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

	itf->puts = flputs;
	itf->data = term;

	return itf;
}

interface_probe("terminal", probe_itf);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term;
	term_flags_t *flags;


	term = (term_t*)dev->data;

	/* read */
	if(term->rx_err)
		goto_errno(err, E_IO);

	critsec_lock(&term->lock);

	if(term->hw->rx_int)	n = ringbuf_read(&term->rx_buf, buf, n);
	else					n = term->hw->gets(buf, n, &term->rx_err, term->hw->data);

	critsec_unlock(&term->lock);

	/* handle terminal flags */
	flags = term->hw->get_flags(term->cfg);

	if(handle_flags(buf, n, 1, flags->iflags, iflag_hdlr, term) == 0x0)
		goto_errno(err, E_IO);

	if(handle_flags(buf, n, n, flags->lflags, lflag_hdlr, term) == 0x0)
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
	flags = term->hw->get_flags(term->cfg);

	_s = handle_flags(s, n, 1, flags->oflags, oflag_hdlr, term);
	n_put = _s - s;

	if(_s == 0x0)
		return 0;

	/* perform write */
	return (puts(_s, n - n_put, term) + n_put == n) ? n : 0;
}

static size_t puts(char const *s, size_t n, term_t *term){
	size_t r;
	tx_data_t data;


	if(n == 0)
		return 0;

	if(n == 1 || int_enabled() == INT_NONE  || term->hw->tx_int == 0){
		critsec_lock(&term->lock);
		r = term->hw->puts(s, n, term->hw->data);
		critsec_unlock(&term->lock);

		return r;
	}

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
		term->rx_err |= TERR_RX_FULL;

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

static char *handle_flags(char *s, size_t n, size_t incr, uint8_t flags, flag_hdlr_t *hdlr, term_t *term){
	size_t i,
		   j,
		   n_put;
	char *x;


	x = s;

	for(i=0; i<n; i+=incr){
		for(j=0; hdlr[j]!=0x0; j++){
			if((flags & (0x1 << j)) == 0)
				continue;

			n_put = x - s;
			x = hdlr[j](x, i - n_put, n - n_put, term);

			if(x == 0x0)
				return 0x0;
		}
	}

	return x;
}

static char *i_crnl(char *s, size_t idx, size_t n, term_t *term){
	if(s[idx] == '\r')
		s[idx] = '\n';

	return s;
}

static char *i_nlcr(char *s, size_t idx, size_t n, term_t *term){
	if(s[idx] == '\n')
		s[idx] = '\r';

	return s;
}

static char *o_crnl(char *s, size_t idx, size_t n, term_t *term){
	if(s[idx] == '\r')
		s[idx] = '\n';

	return s;
}

static char *o_nlcr(char *s, size_t idx, size_t n, term_t *term){
	size_t r;


	if(s[idx] != '\n')
		return s;

	r = puts(s, idx, term);
	r += puts("\r\n", 2, term);

	return (r == idx + 2) ? s + idx + 1 : 0x0;
}

static char *l_echo(char *s, size_t idx, size_t n, term_t *term){
	return (puts(s, n, term) == n) ? s : 0x0;
}
