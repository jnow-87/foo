#ifndef ATMEGA_H
#define ATMEGA_H


#include <config/config.h>
#include <sys/const.h>

#ifdef CONFIG_ATMEGA1284P
#include <arch/avr/atmega1284_register.h>
#endif // CONFIG_ATMEGA1284P

#ifdef CONFIG_ATMEGA88PA
#include <arch/avr/atmega88_register.h>
#endif // CONFIG_ATMEGA88PA

#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

#include <arch/avr/core.h>
#include <arch/avr/uart.h>
#include <arch/avr/register.h>
#include <arch/avr/interrupt.h>
#include <arch/avr/timebase.h>
#include <arch/avr/thread.h>
#include <arch/avr/process.h>
#include <arch/avr/libmain.h>
#include <arch/types.h>
#include <sys/types.h>

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* macros */
// interrupt handling
#if defined(CONFIG_AVR_ATMEGA) || defined(CONFIG_AVR_XMEGA)

#define ICALL				call
#define INT_VEC_WORDS		2

#else

#define ICALL				rcall
#define INT_VEC_WORDS		1

#endif


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

	.thread_context_init = avr_thread_context_init,

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

	/* main entry */
#ifdef KERNEL
	.libmain = 0x0,
#else
	.libmain = avr_libmain,
#endif // KERNEL
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
