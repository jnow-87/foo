/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <kernel/memory.h>
#include <kernel/driver.h>
#include <driver/term.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/uart.h>


/* local/static prototypes */
static int configure(void *cfg, void *data);
static term_flags_t get_flags(void *cfg);
static char putc(char c, void *data);
static size_t putsn(char const *s, size_t n, void *data);
static size_t gets(char *s, size_t n, term_err_t *err, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_itf_t *itf;


	itf = kmalloc(sizeof(term_itf_t));

	if(itf == 0x0)
		return 0x0;

	itf->configure = configure;
	itf->get_flags = get_flags;
	itf->putc = putc;
	itf->puts = putsn;
	itf->gets = gets;
	itf->data = 0x0;
	itf->rx_int = 0;
	itf->tx_int = 0;
	itf->cfg_size = sizeof(uart_cfg_t);

	return itf;
}

interface_probe("x86,uart", probe);

static int configure(void *_cfg, void *data){
	return E_OK;
}

static term_flags_t get_flags(void *cfg){
	return ((uart_cfg_t*)cfg)->flags;
}

static char putc(char c, void *data){
	lnx_write(1, &c, 1);

	return c;
}

static size_t putsn(char const *s, size_t n, void *data){
	if(s == 0x0){
		errno = E_INVAL;
		return 0;
	}

	lnx_write(1, s, n);

	return n;
}

static size_t gets(char *s, size_t n, term_err_t *err, void *data){
	lnx_read_fix(0, s, n);

	return n;
}
