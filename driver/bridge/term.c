/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/bridge.h>
#include <driver/term.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/math.h>
#include <sys/uart.h>


/* local/static prototypes */
static int configure(void *cfg, void *brdg);
static char putc(char c, void *brdg);
static size_t puts(char const *s, size_t n, void *brdg);
static size_t gets(char *s, size_t n, term_err_t *err, void *brdg);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	bridge_t *brdg;
	bridge_ops_t ops;
	bridge_cfg_t *dtd;
	term_itf_t *itf;


	dtd = (bridge_cfg_t*)dt_data;

	ops.read = 0x0;
	ops.write = 0x0;

	// TODO replace null pointers with dummy functions
	brdg = bridge_create(dtd, &ops, 0x0);

	if(brdg == 0x0)
		goto err_0;

	itf = kmalloc(sizeof(term_itf_t));

	if(itf == 0x0)
		goto err_1;

	itf->configure = configure;
	itf->putc = putc;
	itf->puts = puts;
	itf->gets = gets;

	itf->data = brdg;
	itf->rx_int = 0;	// TODO
	itf->tx_int = 0;	// TODO
	itf->cfg_size = sizeof(uart_cfg_t);
	itf->cfg_flags_offset = offsetof(uart_cfg_t, iflags);

	return itf;


err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

interface_probe("bridge,terminal", probe);

static int configure(void *cfg, void *brdg){
	// the bridge hardware configuration must not be changed
	return E_OK;
}

static char putc(char c, void *brdg){
	return (bridge_write(((bridge_t*)brdg)->peer, &c, 1) == 1) ? c : ~c;
}

static size_t puts(char const *s, size_t n, void *brdg){
	size_t i;
	int16_t x;


	for(i=0; i<n; i+=x){
		x = MIN(n - i, 255);
		x = bridge_write(((bridge_t*)brdg)->peer, s + i, x);

		if(x < 0)
			break;
	}

	return i;
}

static size_t gets(char *s, size_t n, term_err_t *err, void *brdg){
	size_t i;
	int16_t x;


	for(i=0; i<n; i+=x){
		x = MIN(n - i, 255);
		x = bridge_read(((bridge_t*)brdg)->peer, s + i, x);

		if(x < 0){
			*err = TERR_UNKNOWN;	// TODO
			break;
		}
	}

	return i;
}
