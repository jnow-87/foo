#ifndef ATMEGA88_H
#define ATMEGA88_H


#include <arch/avr/atmega88_register.h>
#include <sys/const.h>


/* macros */
#define __ATMEGA__

// memory layout
#define MCU_FLASH_SIZE		_8k
#define MCU_SRAM_SIZE		_1k

#define KERNEL_STACK_SIZE	128
#define KERNEL_HEAP_SIZE	128

#define IO_BASE				0x0
#define IO_SIZE				256

#define DATA_SIZE			_1k

// clocking
#define WATCHDOG_CLOCK_HZ	128000


#endif // ATMEGA88_H
