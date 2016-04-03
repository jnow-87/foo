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

#if defined(__ATMEGA__) || defined(__XMEGA__)

// interrupt handling
#define ICALL				call
#define INT_VEC_WORDS		2

#else

// interrupt handling
#define ICALL				rcall
#define INT_VEC_WORDS		1

#endif


#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

#include <arch/avr/register.h>
#include <arch/avr/interrupt.h>
#include <arch/avr/timebase.h>
#include <arch/avr/thread.h>
#include <sys/types.h>

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* macros */
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

// scheduler timer
#define INT_SCHED						INT_WATCHDOG


/* types */
#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

#include <arch/types.h>
#include <arch/avr/core.h>
#include <arch/avr/uart.h>


typedef struct{
	uint16_t core_clock_khz;
	uint16_t timebase_clock_khz;
} arch_info_t;

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* static variables */
#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

// kernel callbacks
#ifdef KERNEL

static const arch_callbacks_kernel_t arch_cbs_kernel = {
	.page_entry_write = 0x0,
	.page_entry_inval_idx = 0x0,
	.page_entry_inval_va = 0x0,
	.page_entry_search = 0x0,

	.copy_from_user = 0x0,
	.copy_to_user = 0x0,

	.int_enable = avr_int_enable,
	.int_enabled = avr_int_enabled,
	.int_hdlr_register = avr_int_hdlr_register,
	.int_hdlr_release = avr_int_hdlr_release,

	.ipi_sleep = 0x0,
	.ipi_wake = 0x0,

	.thread_call = 0x0,			/* TODO */
	.thread_kill = 0x0,			/* TODO */

#ifdef CONFIG_KERNEL_UART
	.putchar = avr_putchar,
	.puts = avr_puts,
#else
	.putchar = 0x0,
	.puts = 0x0,
#endif // CONFIG_KERNEL_UART
};

#endif // KERNEL

// common callbacks
static const arch_callbacks_common_t arch_cbs_common = {
	.timebase = 0x0,			/* TODO */
	.timebase_to_time = 0x0,	/* TODO */

	/* atomics */
	.cas = 0x0,

	/* core */
	.core_id = 0x0,
	.core_sleep = avr_core_sleep,
	.core_halt = avr_core_halt,

	/* syscall */
	.syscall = 0x0,				/* TODO */
};

// architecture info
static const arch_info_t arch_info = {
	.core_clock_khz = CONFIG_CORE_CLOCK_HZ / 1000,
	.timebase_clock_khz = 0x0,
};

#endif // _x86_
#endif // __x86_64__
#endif // ASM


#endif // ATMEGA_H
