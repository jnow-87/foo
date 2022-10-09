/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_BRIDGE_DGRAM_H
#define DRIVER_BRIDGE_DGRAM_H


#include <driver/bridge.h>
#include <sys/types.h>


/* global functions */
bridge_dgram_t *dgram_alloc_rx(bridge_t *brdg);
bridge_dgram_t *dgram_alloc_tx(bridge_t *brdg, void const *buf, uint8_t n);
void dgram_free(bridge_dgram_t *dgram, bridge_t *brdg);

void dgram_init(bridge_dgram_t *dgram, bridge_dgram_type_t type, void *buf, uint8_t n, bridge_t *brdg);
int dgram_init_retry(bridge_dgram_t *dgram);

bridge_dgram_state_t dgram_read(bridge_dgram_t *dgram, bridge_t *brdg);
bridge_dgram_state_t dgram_write(bridge_dgram_t *dgram, bridge_t *brdg);


#endif // DRIVER_BRIDGE_DGRAM_H
