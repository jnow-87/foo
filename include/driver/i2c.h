/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H


#include <sys/i2c.h>


/* types */
typedef struct i2c_itf_t{
	int (*configure)(i2c_cfg_t *cfg, void *data);

	int (*master_read)(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
	int (*master_write)(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
	int (*slave_read)(uint8_t *buf, size_t n, void *data);
	int (*slave_write)(uint8_t *buf, size_t n, void *data);

	void *data;
} i2c_itf_t;


#endif // DRIVER_I2C_H
