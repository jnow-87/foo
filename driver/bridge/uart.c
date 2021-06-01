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
#include <sys/types.h>
#include <sys/errno.h>


/* local/static prototypes */
static int read(bridge_t *brdg, void *buf, size_t n);
static int write(bridge_t *brdg, void const *buf, size_t n);


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	bridge_ops_t ops;
	bridge_cfg_t *dtd;
	term_itf_t *dti;


	dtd = (bridge_cfg_t*)dt_data;
	dti = (term_itf_t*)dt_itf;

	if(dti->configure(&dtd->hw_cfg, dti->data) != 0)
		return 0x0;

	ops.read = read;
	ops.write = write;

	return (bridge_create(dt_data, &ops, dt_itf) == 0x0) ? -errno : E_OK;
}

device_probe("bridge,uart", probe);

static int read(bridge_t *brdg, void *buf, size_t n){
	size_t i,
		   r;
	term_itf_t *hw;
	term_err_t e;


	hw = (term_itf_t*)brdg->hw;
	e = TERR_NONE;

	for(i=0; i<n; i+=r){
		r = hw->gets(buf + i, n - i, &e, hw->data);

		if(e != TERR_NONE)
			return_errno(E_IO);
	}

	return 0;
}

static int write(bridge_t *brdg, void const *buf, size_t n){
	term_itf_t *hw;


	hw = (term_itf_t*)brdg->hw;

	if(hw->puts(buf, n, hw->data) != n)
		return_errno(E_IO);

	return 0;
}

