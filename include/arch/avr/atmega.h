#ifndef ATMEGA_H
#define ATMEGA_H


#include <config/config.h>
#include <sys/const.h>

#ifdef CONFIG_ATMEGA1284P
#include <arch/avr/atmega1284.h>
#endif // CONFIG_ATMEGA1284P

#ifdef CONFIG_ATMEGA88PA
#include <arch/avr/atmega88.h>
#endif // CONFIG_ATMEGA88PA

#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

#include <arch/avr/core.h>
#include <arch/avr/register.h>
#include <arch/avr/interrupt.h>
#include <arch/avr/timebase.h>
#include <arch/avr/thread.h>
#include <arch/avr/process.h>
#include <arch/avr/syscall.h>
#include <arch/avr/lib.h>
#include <arch/types.h>
#include <driver/avr_uart.h>
#include <sys/types.h>

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* macros */
// interrupt handling
#if defined(CONFIG_AVR_ATMEGA) || defined(CONFIG_AVR_XMEGA)

#define XCALL			call
#define XJMP			jmp
#define INT_VEC_SIZE	4

#else

#define XCALL			rcall
#define XJMP			rjmp
#define INT_VEC_SIZE	2

#endif


/* static variables */
#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

// kernel callbacks
#ifdef BUILD_KERNEL

static arch_callbacks_kernel_t const arch_cbs_kernel = {
	/* core */
	.core_panic = avr_core_panic,

	/* virtual memory management */
	.page_entry_write = 0x0,
	.page_entry_inval_idx = 0x0,
	.page_entry_inval_va = 0x0,
	.page_entry_search = 0x0,

	.copy_from_user = 0x0,
	.copy_to_user = 0x0,

	/* interrupts */
	.int_enable = avr_int_enable,
	.int_enabled = avr_int_enabled,

	.ipi_sleep = 0x0,
	.ipi_wake = 0x0,

	/* threading */
	.thread_context_init = avr_thread_context_init,

	/* terminal I/O */
#ifdef CONFIG_AVR_UART
	.putchar = avr_uart_putchar,
	.puts = avr_uart_puts,
#else
	.putchar = 0x0,
	.puts = 0x0,
#endif // CONFIG_KERNEL_UART
};

#endif // BUILD_KERNEL

// common callbacks
static arch_callbacks_common_t const arch_cbs_common = {
	.timebase = 0x0,			/* TODO */
	.timebase_to_time = 0x0,	/* TODO */

	/* atomics */
	.cas = 0x0,

	/* core */
	.core_id = 0x0,
	.core_sleep = avr_core_sleep,

	/* syscall */
#ifdef BUILD_KERNEL
	.sc = 0x0,
#else
	.sc = avr_sc,
#endif // BUILD_KERNEL

	/* main entry */
#ifdef BUILD_KERNEL
	.lib_crt0 = 0x0,
	.lib_main = 0x0,
#else
	.lib_crt0 = avr_lib_crt0,
	.lib_main = avr_lib_main,
#endif // BUILD_KERNEL
};

// architecture info
static arch_info_t const arch_info = {
	.core_clock_khz = CONFIG_CORE_CLOCK_HZ / 1000,
	.timebase_clock_khz = 0x0,
};

#endif // _x86_
#endif // __x86_64__
#endif // ASM


#endif // ATMEGA_H
