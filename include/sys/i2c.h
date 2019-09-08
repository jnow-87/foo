/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_I2C_H
#define SYS_I2C_H


/* types */
typedef enum{
	I2C_MODE_MASTER = 1,
	I2C_MODE_SLAVE
} i2c_mode_t;

typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint8_t mode;		/**< cf. i2c_mode_t */
	uint16_t clock_khz;

	uint8_t bcast_en;
	uint8_t host_addr,
			target_addr;
} i2c_cfg_t;

#endif // SYS_I2C_H
