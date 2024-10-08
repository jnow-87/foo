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
typedef UINT(CONFIG_GPIO_INT_WIDTH) intgpio_t;

typedef struct{
	intgpio_t mask;
	signal_t signum;
} gpio_sig_cfg_t;

typedef struct{
	intgpio_t in_mask,
			  out_mask,
			  int_mask;
} gpio_port_cfg_t;


#endif // SYS_GPIO_H
