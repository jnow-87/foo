#ifndef ATMEGA_H
#define ATMEGA_H


#include <config/config.h>
#include <sys/const.h>


/* mcu-specific header */
#if CONFIG_ATMEGA1284P == 1

#include <arch/avr/atmega1284.h>

#elif CONFIG_ATMEGA88PA == 1

#include <arch/avr/atmega88.h>

#endif // CONFIG_ATMEGA*


#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

#include <arch/avr/register.h>
#include <arch/avr/interrupt.h>
#include <arch/avr/timebase.h>
#include <sys/types.h>

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* macros */
// architecture re-defines
#ifdef PIR
#undef PIR
#endif // PIR

#define PIR	0

// memory layout
#define KERNEL_IMG_BASE					CONFIG_KERNEL_BASE_ADDR
#define KERNEL_IMG_SIZE					(MCU_FLASH_SIZE - KERNEL_IMG_BASE)

#define KERNEL_STACK_BASE				(MCU_SRAM_SIZE - KERNEL_STACK_SIZE)
#define KERNEL_STACK_CORE_BASE(core)	(KERNEL_STACK_BASE + ((core) * KERNEL_STACK_CORE_SIZE))
#define KERNEL_STACK_CORE_SIZE			(KERNEL_STACK_SIZE / CONFIG_NCORES)

#define KERNEL_HEAP_BASE				(KERNEL_STACK_BASE - KERNEL_HEAP_SIZE)

#define PROCESS_BASE					(IO_BASE + IO_SIZE)
#define PROCESS_SIZE					(KERNEL_HEAP_BASE - PROCESS_BASE)

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
