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
#include "loop.h"


/* local/static prototypes */
static char putc(char c, void *data);
static size_t puts(char const *s, size_t n, void *data);
static size_t gets(char *s, size_t n, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_itf_t *itf;
	loop_t *loop;


	itf = kcalloc(1, sizeof(term_itf_t));
	loop = loop_create(dt_data);

	if(itf == 0x0 || loop == 0x0)
		goto err;

	itf->putc = putc;
	itf->puts = puts;
	itf->gets = gets;

	itf->data = loop;
	itf->cfg = dtd;
	itf->cfg_size = sizeof(loop_cfg_t);

	return itf;


err:
	kfree(itf);
	loop_destroy(loop);

	return 0x0;
}

driver_probe("loop,term", probe);

static char putc(char c, void *data){
	if(loop_write(data, &c, 1) != 1)
		return ~c;
	return c;
}

static size_t puts(char const *s, size_t n, void *data){
	return loop_write(data, (void*)s, n);
}

static size_t gets(char *s, size_t n, void *data){
	return loop_read(data, s, n);
}
