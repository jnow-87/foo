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
static size_t gets(char *s, size_t n, term_err_t *err, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd;
	term_itf_t *itf;
	dev_data_t *uart;


	dtd = (dt_data_t*)dt_data;
	itf = kmalloc(sizeof(term_itf_t));
	uart = kmalloc(sizeof(dev_data_t));

	if(itf == 0x0 || uart == 0x0)
		goto err;

	uart->dtd = dtd;
	uart->fd = lnx_open(dtd->path, LNX_O_RDWR, 0666);

	itf->configure = configure;
	itf->putc = putc;
	itf->puts = putsn;
	itf->gets = gets;
	itf->data = uart;
	itf->rx_int = dtd->rx_int;
	itf->tx_int = 0;
	itf->cfg_size = sizeof(uart_cfg_t);
	itf->cfg_flags_offset = offsetof(uart_cfg_t, iflags);

	return itf;


err:
	kfree(uart);
	kfree(itf);

	return 0x0;
}

interface_probe("x86,uart", probe);

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
	if(s == 0x0){
		errno = E_INVAL;
		return 0;
	}

	lnx_write(((dev_data_t*)data)->fd, s, n);

	return n;
}

static size_t gets(char *s, size_t n, term_err_t *err, void *data){
	return lnx_read(((dev_data_t*)data)->fd, s, n);
}
