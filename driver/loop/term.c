/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/term.h>
#include <sys/compiler.h>
#include <sys/errno.h>
#include <sys/loop.h>
#include "loop.h"


/* local/static prototypes */
static int configure(void *cfg, void *data);
static char putc(char c, void *data);
static size_t puts(char const *s, size_t n, void *data);
static size_t gets(char *s, size_t n, term_err_t *err, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_itf_t *itf;
	loop_t *loop;


	itf = kmalloc(sizeof(term_itf_t));
	loop = loop_create(dt_data);

	if(itf == 0x0 || loop == 0x0)
		goto err;

	itf->configure = configure;
	itf->putc = putc;
	itf->puts = puts;
	itf->gets = gets;

	itf->data = loop;
	itf->rx_int = 0;
	itf->tx_int = 0;
	itf->cfg_size = sizeof(loop_cfg_t);
	itf->cfg_flags_offset = offsetof(loop_cfg_t, iflags);

	return itf;


err:
	kfree(itf);
	loop_destroy(loop);

	return 0x0;
}

driver_probe("loop,term", probe);

static int configure(void *cfg, void *data){
	return E_OK;
}

static char putc(char c, void *data){
	if(loop_write(data, &c, 1) != 1)
		return ~c;
	return c;
}

static size_t puts(char const *s, size_t n, void *data){
	return loop_write(data, (void*)s, n);
}

static size_t gets(char *s, size_t n, term_err_t *err, void *data){
	return loop_read(data, s, n);
}
