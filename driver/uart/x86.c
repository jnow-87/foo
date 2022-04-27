/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/memory.h>
#include <kernel/driver.h>
#include <driver/term.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/uart.h>


/* types */
typedef struct{
	char const *path;
	uint8_t const rx_int;
} dt_data_t;

typedef struct{
	dt_data_t *dtd;
	int fd;
} dev_data_t;


/* local/static prototypes */
static int configure(void *cfg, void *data);
static char putc(char c, void *data);
static size_t putsn(char const *s, size_t n, void *data);
static size_t gets(char *s, size_t n, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd;
	term_itf_t *itf;
	dev_data_t *uart;


	dtd = (dt_data_t*)dt_data;
	itf = kmalloc(sizeof(term_itf_t));
	uart = kmalloc(sizeof(dev_data_t));

	if(itf == 0x0 || uart == 0x0)
		goto err_0;

	uart->dtd = dtd;
	uart->fd = lnx_open(dtd->path, LNX_O_RDWR, 0666);

	// make file operations non-blocking to ensure gets() doesn't block
	if(lnx_fcntl(uart->fd, LNX_F_SETFL, LNX_O_NONBLOCK) != 0)
		goto err_1;

	itf->configure = configure;
	itf->putc = putc;
	itf->puts = putsn;
	itf->gets = gets;
	itf->error = 0x0;
	itf->data = uart;
	itf->rx_int = dtd->rx_int;
	itf->tx_int = 0;
	itf->cfg_size = sizeof(uart_cfg_t);
	itf->cfg_flags_offset = offsetof(uart_cfg_t, iflags);

	return itf;


err_1:
	lnx_close(uart->fd);

err_0:
	kfree(uart);
	kfree(itf);

	return 0x0;
}

driver_probe("x86,uart", probe);

static int configure(void *cfg, void *data){
	dev_data_t *uart;
	x86_hw_op_t op;


	uart = (dev_data_t*)data;

	op.num = HWO_UART_CFG;

	op.uart.int_num = uart->dtd->rx_int;
	memcpy(&op.uart.cfg, cfg, sizeof(uart_cfg_t));
	strncpy(op.uart.path, uart->dtd->path, 64);
	op.uart.path[63] = 0;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);

	return E_OK;
}

static char putc(char c, void *data){
	lnx_write(((dev_data_t*)data)->fd, &c, 1);

	return c;
}

static size_t putsn(char const *s, size_t n, void *data){
	if(s == 0x0)
		goto_errno(err, E_INVAL);

	lnx_write(((dev_data_t*)data)->fd, s, n);

	return n;


err:
	return 0;
}

static size_t gets(char *s, size_t n, void *data){
	ssize_t r;


	r = lnx_read(((dev_data_t*)data)->fd, s, n);

	if(r < 0)
		goto_errno(err, E_IO);

	return r;


err:
	return 0;
}
