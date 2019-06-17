/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arm/atmel/pio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/register.h>


/* macros */
#define PER		0x0
#define PDR		0x4
#define PSR		0x8

#define ABCDSR1	0x70
#define ABCDSR2	0x74

#define OER		0x10
#define ODR		0x14
#define OSR		0x18


/* static variables */
static void *port_base_addr[] = {
	(void*)0x400e0e00,
	(void*)0x400e1000,
	(void*)0x400e1200,
	(void*)0x400e1400,
	(void*)0x400e1600
};


/* global functions */
void pio_pin_enable(pio_port_t port, uint8_t pin, pio_func_t func){
	mreg_w(port_base_addr[port] + ABCDSR1, (func & 0x1) << pin);
	mreg_w(port_base_addr[port] + ABCDSR2, (func & 0x2) << (pin - 1));
	mreg_w(port_base_addr[port] + PER, 0x1 << pin);
}

void pio_pin_disable(pio_port_t port, uint8_t pin, pio_func_t func){
	mreg_w(port_base_addr[port] + PDR, 0x1 << pin);
}

void pio_pin_set(pio_port_t port, uint8_t pin){
	mreg_w(port_base_addr[port] + OER, 0x1 << pin);
}

void pio_pin_clear(pio_port_t port, uint8_t pin){
	mreg_w(port_base_addr[port] + ODR, 0x1 << pin);
}
