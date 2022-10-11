/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/kprintf.h>
#include <driver/bridge.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/string.h>


/* local/static prototypes */
static int rw(bridge_t *brdg, void *buf, size_t n, int16_t (*op)(bridge_t *, void *, uint8_t));


/* global functions */
int i2cbrdg_read(bridge_t *brdg, void *buf, size_t n){
	return rw(brdg, buf, n, bridge_read);
}

int i2cbrdg_write(bridge_t *brdg, void *buf, size_t n){
	return rw(brdg, buf, n, bridge_write);
}


/* local functions */
static int rw(bridge_t *brdg, void *buf, size_t n, int16_t (*op)(bridge_t *, void *, uint8_t)){
	int16_t r;


	if(n > 255)
		return_errno(E_LIMIT);

	r = op(brdg, buf, n);

	DEBUG("i2cbrdg %s %zu/%zu: %s\n", (op == bridge_read) ? "read" : "write", r, n, strerror((r < 0) ? -r : 0));

	if(r < 0)
		return_errno(-r);

	if(r != (int16_t)n)
		return_errno(E_AGAIN);

	return 0;
}
