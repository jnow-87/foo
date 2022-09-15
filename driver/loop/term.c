/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/term.h>
#include <sys/errno.h>
#include <sys/loop.h>
#include <sys/ringbuf.h>


/* local/static prototypes */
static char putc(char c, void *data);
static size_t puts(char const *s, size_t n, void *data);
static size_t gets(char *s, size_t n, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_itf_t *itf;
	ringbuf_t *loop;
	loop_cfg_t *dtd;


	dtd = (loop_cfg_t*)dt_data;

	itf = kcalloc(1, sizeof(term_itf_t));
	loop = kmalloc(sizeof(ringbuf_t) + dtd->size);

	if(itf == 0x0 || loop == 0x0)
		goto err;

	ringbuf_init(loop, (char*)loop + sizeof(ringbuf_t), dtd->size);

	itf->putc = putc;
	itf->puts = puts;
	itf->gets = gets;

	itf->data = loop;
	itf->cfg = dtd;
	itf->cfg_size = sizeof(loop_cfg_t);

	return itf;


err:
	kfree(loop);
	kfree(itf);

	return 0x0;
}

driver_probe("loop,term", probe);

static char putc(char c, void *data){
	if(ringbuf_write(data, &c, 1) != 1)
		return ~c;
	return c;
}

static size_t puts(char const *s, size_t n, void *data){
	return ringbuf_write(data, (void*)s, n);
}

static size_t gets(char *s, size_t n, void *data){
	n = ringbuf_read(data, s, n);

	if(n == 0)
		goto_errno(err, E_END);

	return n;


err:
	return 0;
}
