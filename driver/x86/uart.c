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
#include <kernel/interrupt.h>
#include <driver/term.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/uart.h>


/* types */
typedef struct{
	char const *path;
	uint8_t const rx_int,
				  tx_int;

	uart_cfg_t cfg;
} __packed dt_data_t;

typedef struct{
	dt_data_t *dtd;
	int fd;
} dev_data_t;


/* local/static prototypes */
static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw);
static char putc(char c, void *hw);
static size_t putsn(char const *s, size_t n, void *hw);
static size_t gets(char *s, size_t n, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	term_itf_t *itf;
	dev_data_t *uart;


	itf = kcalloc(1, sizeof(term_itf_t));
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

	itf->hw = uart;
	itf->cfg = &dtd->cfg;
	itf->cfg_size = sizeof(uart_cfg_t);
	itf->rx_int = dtd->rx_int;
	itf->tx_int = dtd->tx_int;

	return itf;


err_1:
	lnx_close(uart->fd);

err_0:
	kfree(uart);
	kfree(itf);

	return 0x0;
}

driver_probe("x86,uart", probe);

static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw){
	dev_data_t *uart = (dev_data_t*)hw;
	x86_hw_op_t op;


	op.num = HWO_UART_CFG;

	op.uart.int_num = uart->dtd->rx_int;
	memcpy(&op.uart.cfg, hw_cfg, sizeof(uart_cfg_t));
	strncpy(op.uart.path, uart->dtd->path, 64);
	op.uart.path[63] = 0;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);

	return 0;
}

static char putc(char c, void *hw){
	return (putsn(&c, 1, hw) != 1) ? ~c : c;
}

static size_t putsn(char const *s, size_t n, void *hw){
	dev_data_t *uart = (dev_data_t*)hw;


	lnx_write(uart->fd, s, n);

	// the x86 hardware simulated by the test framework doesn't support
	// tx interrupts, hence a fake interrupt is produced here
	if(uart->dtd->tx_int)
		int_foretell(uart->dtd->tx_int);

	return n;


err:
	return 0;
}

static size_t gets(char *s, size_t n, void *hw){
	ssize_t r;


	r = lnx_read(((dev_data_t*)hw)->fd, s, n);

	if(r < 0){
		switch(r){
		case -11: // EAGAIN
			lnx_nanosleep(1000000);
			goto_errno(err, E_AGAIN);

		default:
			goto_errno(err, E_IO);
		}
	}

	return r;


err:
	return 0;
}
