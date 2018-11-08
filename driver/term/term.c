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
#include <driver/term.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>


/* local/static prototypes */
static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);


/* global functions */
term_t *term_register(char const *suffix, term_ops_t *ops, void *data){
	char name[strlen(suffix) + 4];
	void *rx_buf;
	devfs_dev_t *dev;
	devfs_ops_t dev_ops;
	term_t *term;


	/* allocate terminal */
	term = kmalloc(sizeof(term_t));

	if(term == 0x0)
		goto_errno(err_0, E_NOMEM);

	/* allocate recv buffer */
	rx_buf = kmalloc(CONFIG_TERM_RXBUF_SIZE);

	if(rx_buf == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* register device */
	strcpy(name, "tty");
	strcpy(name + 3, suffix);

	dev_ops.open = 0x0;
	dev_ops.close = 0x0;
	dev_ops.read = read;
	dev_ops.write = write;
	dev_ops.ioctl = ioctl;
	dev_ops.fcntl = 0x0;

	dev = devfs_dev_register(name, &dev_ops, term);

	if(dev == 0x0)
		goto err_2;

	/* init term struct */
	memset(&term->cfg, 0x0, sizeof(term_cfg_t));

	term->ops = *ops;
	term->data = data;
	term->dev = dev;

	term->rx_rdy = &dev->node->rd_sig;
	term->rx_err = TE_NONE;

	ringbuf_init(&term->rx_buf, rx_buf, CONFIG_TERM_RXBUF_SIZE);

	return term;


err_2:
	kfree(rx_buf);

err_1:
	kfree(term);

err_0:
	return 0x0;
}

void term_release(term_t *term){
	devfs_dev_release(term->dev);

	kfree(term->rx_buf.data);
	kfree(term);
}


/* local functions */
static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	int r;
	term_t *term;


	term = (term_t*)dev->data;

	/* read */
	if(term->rx_err){
		errno = E_IO;
		return 0;
	}

	r = ringbuf_read(&term->rx_buf, buf, n);

	/* handle terminal flags */
	// handle TF_ECHO
	if(r > 0 && (term->cfg.flags & TF_ECHO)){
		if(term->ops.puts(term, buf, r) != E_OK)
			return 0;
	}

	return r;
}

static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	term_t *term;


	term = (term_t*)dev->data;

	if(term->ops.puts(term, buf, n) != E_OK)
		return 0;
	return n;
}

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	term_t *term;


	term = (term_t*)dev->data;

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, &term->cfg, sizeof(term_cfg_t));
		return E_OK;

	case IOCTL_CFGWR:
		if(term->ops.config(term, (term_cfg_t*)data) != E_OK)
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
