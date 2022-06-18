/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_GPIO_H
#define SYS_GPIO_H


#include <config/config.h>
#include <sys/types.h>
#include <sys/signal.h>


/* types */
typedef UINT(CONFIG_GPIO_INT_WIDTH) gpio_int_t;

typedef struct{
	gpio_int_t mask;
	signal_t sig;
} gpio_int_cfg_t;


#endif // SYS_GPIO_H
