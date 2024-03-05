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
#include <sys/errno.h>
#include <sys/math.h>
#include <sys/uart.h>


/* local/static prototypes */
static size_t puts(char const *s, size_t n, bool blocking, void *brdg);
static size_t gets(char *s, size_t n, void *brdg);
static errno_t error(void *brdg);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	bridge_cfg_t *dtd = (bridge_cfg_t*)dt_data;
	bridge_t *brdg;
	term_itf_t *itf;


	brdg = bridge_create(0x0, dtd, 0x0);

	if(brdg == 0x0)
		goto err_0;

	itf = kcalloc(1, sizeof(term_itf_t));

	if(itf == 0x0)
		goto err_1;

	itf->puts = puts;
	itf->gets = gets;
	itf->error = error;

	itf->hw = brdg;
	itf->cfg = 0x0;
	itf->cfg_size = 0;
	itf->rx_int = dtd->rx_int;
	itf->tx_int = dtd->tx_int;

	return itf;


err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

driver_probe("bridge,uart-dev", probe);

static size_t puts(char const *s, size_t n, bool blocking, void *brdg){
	size_t i;
	int16_t x;


	for(i=0; i<n; i+=x){
		x = MIN(n - i, 255u);
		x = bridge_write(brdg, (char*)s + i, x);

		if(x <= 0)
			break;
	}

	return i;
}

static size_t gets(char *s, size_t n, void *brdg){
	size_t i;
	int16_t x;


	for(i=0; i<n; i+=x){
		x = MIN(n - i, 255u);
		x = bridge_read(brdg, s + i, x);

		if(x < 0)
			goto_errno(err, E_IO);

		if(x == 0)
			break;
	}

	return i;


err:
	return 0;
}

static errno_t error(void *brdg){
	bridge_t *peer = ((bridge_t*)brdg)->peer;
	errno_t ecode;


	ecode = peer->errno;
	peer->errno = 0;

	return ecode;
}
