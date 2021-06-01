/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_BRIDGE_H
#define DRIVER_BRIDGE_H


#include <sys/types.h>


/* incomplete types */
struct bridge_t;


/* types */
typedef struct{
	uint8_t id;
	uint8_t chunksize_e;

	uint8_t rx_int,
			tx_int;

	uint8_t hw_cfg[];
} bridge_cfg_t;

typedef struct{
	int (*read)(struct bridge_t *brdg, void *buf, size_t n);
	int (*write)(struct bridge_t *brdg, void const *buf, size_t n);
} bridge_ops_t;

typedef struct bridge_t{
	struct bridge_t *prev,
					*next;

	bridge_cfg_t *cfg;
	bridge_ops_t ops;
	void *hw;
	struct bridge_t *peer;

	uint8_t seq_num;
} bridge_t;


/* prototypes */
bridge_t *bridge_create(bridge_cfg_t *cfg, bridge_ops_t *ops, void *hw_itf);
void bridge_destroy(bridge_t *brdg);

int16_t bridge_read(bridge_t *brdg, void *data, uint8_t n);
int16_t bridge_write(bridge_t *brdg, void const *data, uint8_t n);


#endif // DRIVER_BRIDGE_H
