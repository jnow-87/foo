/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_SPI_H
#define SYS_SPI_H


#include <sys/types.h>


/* types */
typedef enum{
	SPI_DEV_SLAVE = 0,
	SPI_DEV_MASTER
} spi_dev_mode_t;

typedef enum{
	SPI_SAMPLE_MODE_0 = 0,		// CPOL = 0, CPHA = 0
	SPI_SAMPLE_MODE_1,			// CPOL = 0, CPHA = 1
	SPI_SAMPLE_MODE_2,			// CPOL = 1, CPHA = 0
	SPI_SAMPLE_MODE_3			// CPOL = 1, CPHA = 1
} spi_sample_mode_t;

typedef enum{
	SPI_MSB_FIRST = 0,
	SPI_LSB_FIRST
} spi_data_order_t;

typedef enum{
	SPI_PRES_2 = 0,
	SPI_PRES_4,
	SPI_PRES_8,
	SPI_PRES_16,
	SPI_PRES_32,
	SPI_PRES_64,
	SPI_PRES_128,
} spi_pres_t;

typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint8_t dev_mode;		/**< cf. spi_dev_mode_t */

	uint8_t sample_mode;	/**< cf. spi_sample_mode_t */
	uint8_t data_order;		/**< cf. spi_data_order_t */
	uint8_t prescaler;		/**< cf. spi_pres_t */
} spi_cfg_t;


#endif // SYS_SPI_H
