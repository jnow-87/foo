/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_SENSOR_H
#define SYS_SENSOR_H


#include <sys/types.h>


/* types */
typedef struct{
	int16_t degc;
	uint16_t mdegc;
} temperature_t;

typedef struct{
	uint32_t pa;
} pressure_t;

typedef struct{
	uint8_t perc;
} humidity_t;

typedef struct{
	temperature_t temp;
	pressure_t press;
	humidity_t hum;
} envsensor_t;


#endif // SYS_SENSOR_H
