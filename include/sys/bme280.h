/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_BME280_H
#define SYS_BME280_H


#include <sys/types.h>


/* types */
typedef enum{
	BME_OSRS_NONE = 0,
	BME_OSRS_1,
	BME_OSRS_2,
	BME_OSRS_4,
	BME_OSRS_8,
	BME_OSRS_16,
} bme280_oversampling_t;

typedef enum{
	BME_STB_500 = 0,
	BME_STB_62500,
	BME_STB_125000,
	BME_STB_500000,
	BME_STB_1000000,
	BME_STB_10000,
	BME_STB_20000,
} bme280_standby_t;

typedef enum{
	BME_FLT_OFF = 0,
	BME_FLT_2,
	BME_FLT_4,
	BME_FLT_8,
	BME_FLT_16,
} bme280_filter_t;

typedef enum{
	BME_MODE_FORCED = 1,
	BME_MODE_NORMAL = 3,
} bme280_mode_t;

typedef enum{
	BME_FTR_TEMP = 0x1,
	BME_FTR_PRESS = 0x2,
	BME_FTR_HUM = 0x4,
} bme280_feature_t;

typedef struct{
	uint8_t mode;				/**< cf. bme280_mode_t */
	uint8_t features;			/**< cf. bme280_feature_t */

	uint8_t standby_time_us;	/**< cf. bme280_standby_t */
	uint8_t filter;				/**< cf. bme280_filter_t */
	uint8_t oversampling_temp,	/**< cf. bme280_oversampling_t */
			oversampling_press,
			oversampling_hum;
} bme280_cfg_t;


#endif // SYS_BME280_H
