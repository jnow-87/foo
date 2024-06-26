/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ATMEGA_H
#define ATMEGA_H


#include <config/config.h>
#include <arch/types.h>

#ifdef CONFIG_ATMEGA1284P
# include <arch/avr/atmega1284.h>
#endif // CONFIG_ATMEGA1284P

#ifndef ASM
# ifndef BUILD_HOST
#  include <sys/compiler.h>
#  include <sys/devicetree.h>
#  include <sys/devtree.h>
#  include <sys/syscall.h>
#  include <sys/thread.h>
#  include <sys/types.h>
# endif // BUILD_HOST
#endif // ASM


/* macros */
#define AVR_SYSTEM_CLOCK_HZ	(((avr_platform_cfg_t*)devtree_arch_payload("avr,platform"))->system_clock_hz)
#define AVR_CPU_CLOCK_HZ	AVR_SYSTEM_CLOCK_HZ
#define AVR_IO_CLOCK_HZ		AVR_SYSTEM_CLOCK_HZ
#define AVR_ADC_CLOCK_HZ	AVR_SYSTEM_CLOCK_HZ
#define AVR_ASYNC_CLOCK_HZ	AVR_SYSTEM_CLOCK_HZ
#define AVR_FLASH_CLOCK_HZ	AVR_SYSTEM_CLOCK_HZ

#if defined(CONFIG_AVR_ISA_AVR51) || defined(CONFIG_AVR_XMEGA)
# define XCALL				call
# define XJMP				jmp
# define XCALL_LEN			4
# define INT_VEC0_LEN		4
# define INT_VEC1_LEN		8
#else
# define XCALL				rcall
# define XJMP				rjmp
# define XCALL_LEN			2
# define INT_VEC0_LEN		2
# define INT_VEC1_LEN		4
#endif


/* incomplete types */
#ifndef ASM
# ifndef BUILD_HOST
struct thread_t;
# endif // BUILD_HOST
#endif // ASM


/* types */
#ifndef ASM
# ifndef BUILD_HOST
typedef struct{
	uint32_t system_clock_hz;
	uint8_t system_clock_prescaler;

	uint8_t watchdog_prescaler;
} avr_platform_cfg_t;

// NOTE when changing thread_ctx_t also check if modifications
// 		to the interrupt service routine are required
typedef struct thread_ctx_t{
	struct thread_ctx_t *next,
						*this;

	uint8_t type;				/**< cf. thread_ctx_type_t */

	uint8_t sreg,				/**< status register */
			mcusr,				/**< control register */
			rampz;				/**< extended Z-pointer */

	uint8_t gpior[3];			/**< GPIO registers */
	uint8_t gpr[32];			/**< general purpose registers */

	void *int_vec_addr,			/**< level-1 interrupt vector return address */
		 *ret_addr;				/**< thread return address on interrupt */
} thread_ctx_t;

STATIC_ASSERT(sizeof(((thread_ctx_t*)(0))->next) == (DEVTREE_ARCH_ADDR_WIDTH == 16 ? 2 : 1));
STATIC_ASSERT(sizeof(((thread_ctx_t*)(0))->type) == 1);
# endif // BUILD_HOST
#endif // ASM


/* prototypes */
#ifndef ASM
# ifndef BUILD_HOST
#  ifdef BUILD_KERNEL
// core
void avr_core_sleep(void);
void avr_core_panic(thread_ctx_t const *tc);

// interrupt
bool avr_int_enable(bool en);
bool avr_int_enabled(void);

void avr_iovfl_hdlr(struct thread_ctx_t *tc);

// thread
void avr_thread_ctx_init(thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg);

// syscall
sc_t *avr_sc_arg(struct thread_t *this_t);
#  endif // BUILD_KERNEL

int avr_atomic(atomic_t op, void *param);

int avr_sc(sc_num_t num, void *param, size_t psize);
# endif // BUILD_HOST
#endif // ASM


/* static variables */
#ifndef ASM
# ifndef BUILD_HOST
#  ifdef BUILD_KERNEL
// kernel ops
static arch_ops_kernel_t const arch_ops_kernel = {
	/* core */
	.core_id = 0x0,
	.core_sleep = avr_core_sleep,
	.core_panic = avr_core_panic,
	.cores_boot = 0x0,

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

	.ipi_int = 0x0,
	.ipi_arg = 0x0,

	/* threading */
	.thread_ctx_init = avr_thread_ctx_init,

	/* syscall */
	.sc_arg = avr_sc_arg,
};
#  endif // BUILD_KERNEL

// common ops
static arch_ops_common_t const arch_ops_common = {
	/* atomics */
	.atomic = avr_atomic,
	.cas = 0x0,
	.atomic_add = 0x0,

	/* syscall */
	.sc = avr_sc,
};
# endif // BUILD_HOST
#endif // ASM


#endif // ATMEGA_H
