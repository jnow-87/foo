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
static size_t puts(char const *s, size_t n, bool blocking, void *hw);
static size_t gets(char *s, size_t n, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	loop_cfg_t *dtd = (loop_cfg_t*)dt_data;
	term_itf_t *itf;
	ringbuf_t *loop;


	itf = kcalloc(1, sizeof(term_itf_t));
	loop = kmalloc(sizeof(ringbuf_t) + dtd->size);

	if(itf == 0x0 || loop == 0x0)
		goto err;

	ringbuf_init(loop, (char*)loop + sizeof(ringbuf_t), dtd->size);

	itf->puts = puts;
	itf->gets = gets;

	itf->hw = loop;
	itf->cfg = dtd;
	itf->cfg_size = sizeof(loop_cfg_t);

	return itf;


err:
	kfree(loop);
	kfree(itf);

	return 0x0;
}

driver_probe("loop,term", probe);

static size_t puts(char const *s, size_t n, bool blocking, void *hw){
	return ringbuf_write(hw, (void*)s, n);
}

static size_t gets(char *s, size_t n, void *hw){
	n = ringbuf_read(hw, s, n);

	if(n == 0)
		goto_errno(err, E_END);

	return n;


err:
	return 0;
}
