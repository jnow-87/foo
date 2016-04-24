#ifndef ATMEGA1284_H
#define ATMEGA1284_H


#include <arch/avr/atmega1284_register.h>
#include <sys/const.h>


/* macros */
#define __ATMEGA__

// memory layout
#define MCU_FLASH_SIZE		_128k
#define MCU_SRAM_SIZE		_16k

#define KERNEL_STACK_SIZE	_1k
#define KERNEL_HEAP_SIZE	_4k

#define IO_BASE				0x0
#define IO_SIZE				256

#define DATA_SIZE			_1k

// clocking
#define WATCHDOG_CLOCK_HZ	128000


#endif // ATMEGA1284_H
