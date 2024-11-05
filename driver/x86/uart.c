/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <kernel/memory.h>
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
	int fd; /**< set by the driver */

	uart_cfg_t cfg;
} dt_data_t;


/* local/static prototypes */
static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw);
static size_t puts(char const *s, size_t , bool blockingn, void *hw);
static size_t gets(char *s, size_t n, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	term_itf_t *itf;


	itf = kcalloc(1, sizeof(term_itf_t));

	if(itf == 0x0)
		goto err_0;

	dtd->fd = lnx_open(dtd->path, LNX_O_RDWR, 0666);

	if(dtd->fd < 0){
		WARN("unable to open uart at \"%s\": lnx-errno=%d\n", dtd->path, -dtd->fd);
		goto_errno(err_0, E_IO);
	}

	// make file operations non-blocking to ensure gets() doesn't block
	if(lnx_fcntl(dtd->fd, LNX_F_SETFL, LNX_O_NONBLOCK) != 0)
		goto_errno(err_1, E_IO);

	itf->configure = configure;
	itf->puts = puts;
	itf->gets = gets;

	itf->hw = dtd;
	itf->cfg = &dtd->cfg;
	itf->cfg_size = sizeof(uart_cfg_t);
	itf->rx_int = dtd->rx_int;
	itf->tx_int = dtd->tx_int;

	return itf;


err_1:
	lnx_close(dtd->fd);

err_0:
	kfree(itf);

	return 0x0;
}

driver_probe("x86,uart", probe);

static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;


	x86_hw_uart_cfg(dtd->path, dtd->rx_int, hw_cfg);

	return 0;
}

static size_t puts(char const *s, size_t n, bool blocking, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;


	lnx_write(dtd->fd, s, n);

	// the x86 hardware simulated by the test framework doesn't support
	// tx interrupts, hence a fake interrupt is produced here
	if(dtd->tx_int)
		int_foretell(dtd->tx_int);

	return n;
}

static size_t gets(char *s, size_t n, void *hw){
	ssize_t r;


	r = lnx_read(((dt_data_t*)hw)->fd, s, n);

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
