/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef BRIDGE_I2C_H
#define BRIDGE_I2C_H


#include <config/config.h>
#include <driver/bridge.h>
#include <sys/compiler.h>
#include <sys/types.h>


/* types */
typedef struct{
	uint8_t cmd;
	uint8_t slave;
	uint8_t nbuf,
			len;
	uint8_t buf[CONFIG_BRIDGE_I2C_INLINE_DATA];
} __packed i2cbrdg_hdr_t;


/* prototypes */
int i2cbrdg_read(bridge_t *brdg, void *buf, size_t n);
int i2cbrdg_write(bridge_t *brdg, void *buf, size_t n);


#endif // BRIDGE_I2C_H
