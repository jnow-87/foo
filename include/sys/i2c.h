/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_I2C_H
#define SYS_I2C_H


#include <sys/types.h>


/* types */
typedef enum{
	I2C_MASTER = 1,
	I2C_SLAVE,
} i2c_mode_t;

typedef struct{
	i2c_mode_t mode;
	uint8_t slave;
} i2c_dev_cfg_t;


#endif // SYS_I2C_H
