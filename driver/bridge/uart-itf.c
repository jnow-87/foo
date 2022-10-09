/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <driver/bridge.h>
#include <driver/term.h>
#include <sys/types.h>
#include <sys/errno.h>


/* local/static prototypes */
static int readb(uint8_t *b, void *hw);
static int writeb(uint8_t b, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	bridge_cfg_t *dtd = (bridge_cfg_t*)dt_data;
	term_itf_t *dti = (term_itf_t*)dt_itf;
	bridge_ops_t ops;
	term_cfg_t term;


	if(dti->configure != 0x0 && dti->configure(&term, dti->cfg, dti->hw) != 0)
		return 0x0;

	ops.readb = readb;
	ops.writeb = writeb;

	dtd->rx_int = dti->rx_int;
	dtd->tx_int = dti->tx_int;

	(void)bridge_create(&ops, dtd, dti);

	return 0x0;
}

driver_probe("bridge,uart-itf", probe);

static int readb(uint8_t *b, void *hw){
	term_itf_t *term = (term_itf_t*)hw;


	if(term->gets((char*)b, 1, term->hw) == 0)
		return_errno(errno ? errno : E_AGAIN);

	return 0;
}

static int writeb(uint8_t b, void *hw){
	term_itf_t *term = (term_itf_t*)hw;


	if(term->puts((char*)&b, 1, term->hw) != 1)
		return_errno(E_IO);

	return 0;
}
