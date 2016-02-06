#ifndef ATMEGA_H
#define ATMEGA_H

#include <config/config.h>

/* mcu-specific header */
#if CONFIG_ATMEGA1284P == 1
#include <arch/avr/atmega1284.h>

#elif CONFIG_ATMEGA88PA == 1
#include <arch/avr/atmega88.h>

#endif // CONFIG_ATMEGA*


#ifndef _x86_
#ifndef __x86_64__

#include <arch/avr/register.h>
#include <arch/avr/timebase.h>
#include <sys/types.h>

#endif // __x86_64__
#endif // _x86_


/* macros */
// memory layout
#define KERNEL_IMG_BASE					0x0
#define KERNEL_IMG_SIZE					0x0

#define KERNEL_STACK_BASE				0x0
#define KERNEL_STACK_SIZE				0x0
#define KERNEL_STACK_CORE_BASE(core)	0x0
#define KERNEL_STACK_CORE_SIZE			0x0

#define KERNEL_HEAP_BASE				0x0
#define KERNEL_HEAP_SIZE				0x0

#define PROCESS_BASE					0x0
#define PROCESS_SIZE					0x0

#define IO_BASE							0x0
#define IO_SIZE							0x0

#define RAMFS_BASE						0x0
#define RAMFS_SIZE						0x0


/* types */
#ifndef _x86_
#ifndef __x86_64__
#ifndef ASM

typedef struct{
	uint16_t core_clock_khz;
	uint16_t timebase_clock_khz;
} arch_info_t;

#endif // ASM
#endif // __x86_64__
#endif // _x86_

#endif // ATMEGA_H
