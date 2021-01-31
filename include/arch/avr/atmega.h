/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ATMEGA_H
#define ATMEGA_H


#include <config/config.h>
#include <sys/const.h>

#ifdef CONFIG_ATMEGA1284P
#include <arch/avr/atmega1284.h>
#endif // CONFIG_ATMEGA1284P

#ifndef ASM
#ifndef BUILD_HOST

#include <config/avrconfig.h>
#include <arch/arch.h>
#include <arch/avr/core.h>
#include <arch/avr/register.h>

#ifdef BUILD_KERNEL
#include <arch/avr/interrupt.h>
#endif // BUILD_KERNEL

#include <arch/avr/thread.h>
#include <arch/avr/syscall.h>
#include <arch/avr/atomic.h>
#include <sys/types.h>

#endif // BUILD_HOST
#endif // ASM


/* macros */
// clocks
#define AVR_CPU_CLOCK_HZ	CONFIG_SYSTEM_CLOCK_HZ
#define AVR_IO_CLOCK_HZ		CONFIG_SYSTEM_CLOCK_HZ
#define AVR_ADC_CLOCK_HZ	CONFIG_SYSTEM_CLOCK_HZ
#define AVR_ASYNC_CLOCK_HZ	CONFIG_SYSTEM_CLOCK_HZ
#define AVR_FLASH_CLOCK_HZ	CONFIG_SYSTEM_CLOCK_HZ

// interrupt handling
#define NUM_INT				(NUM_HW_INT + 2)	// +2 for the pseude interrupts (syscall, instruction overflow)

#if defined(CONFIG_AVR_ISA_AVR51) || defined(CONFIG_AVR_XMEGA)

#define XCALL				call
#define XJMP				jmp
#define INT_VEC_SIZE		4

#else

#define XCALL				rcall
#define XJMP				rjmp
#define INT_VEC_SIZE		2

#endif


/* static variables */
#ifndef ASM
#ifndef BUILD_HOST

// kernel callbacks
#ifdef BUILD_KERNEL

static arch_callbacks_kernel_t const arch_cbs_kernel = {
	/* core */
	.core_id = 0x0,
	.core_sleep = avr_core_sleep,
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

	.int_ipi = 0x0,

	/* threading */
	.thread_context_init = avr_thread_context_init,
	.thread_context_type = avr_thread_context_type,
};

#endif // BUILD_KERNEL

// common callbacks
static arch_callbacks_common_t const arch_cbs_common = {
	/* atomics */
	.cas = avr_cas,
	.atomic_inc = avr_atomic_inc,

	/* syscall */
	.sc = avr_sc,
};

// architecture info
static arch_info_t const arch_info = {
	.kernel_timer_err_us = AVRCONFIG_KTIMER_ERROR_US,
	.sched_timer_err_us = AVRCONFIG_SCHED_ERROR_US,
};

#endif // BUILD_HOST
#endif // ASM


#endif // ATMEGA_H
