/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_V6M_H
#define ARM_V6M_H


#include <kernel/interrupt.h>
#include <sys/compiler.h>
#include <sys/register.h>
#include <sys/syscall.h>
#include <sys/thread.h>
#include <sys/types.h>


/* macros */
// exceptions
#define INT_RESET		1
#define INT_NMI			2
#define INT_HARDFAULT	3
#define INT_SVCALL		11
#define INT_PENDSV		14
#define INT_SYSTICK		15
#define INT_EXT_BASE	16

// memory map
#define AV6M_PPB_BASE	0xe0000000

// ppb register offsets
#define ICSR			0xed04
#define VTOR			0xed08

#define SCR				0xed10

#define SHPR2			0xed1c
#define SHPR3			0xed20

// register bits
#define ICSR_PENDSVSET	28
#define ICSR_PENDSVCLR	27

#define SCR_SEVONPEND	4
#define SCR_SLEEPDEEP	2
#define SCR_SLEEPONEXIT	1

#define SHPR2_PRI11		30
#define SHPR3_PRI15		30
#define SHPR3_PRI14		22

// special register access
#define mrs(reg)({ \
	register register_t v; \
	\
	\
	asm volatile( \
		"mrs %[v], " STR(reg) \
		: [v] "=r" (v) \
	); \
	\
	v; \
})

#define msr(reg, val) \
	asm volatile( \
		"msr " STR(reg) ", %[v]" \
		: \
		: [v] "r" (val) \
	)

#define ppb_write(ppb_offset, val) \
	asm volatile( \
		"str	%[v], [%[reg]]\n" \
		"dsb\n" \
		"isb\n" \
		: \
		: [reg] "r" (AV6M_PPB_BASE | ppb_offset), [v] "r" (val) \
		: "memory" \
	)

#define ppb_read(ppb_offset)	MREG(AV6M_PPB_BASE | ppb_offset)

// synchronisation
#define dmb()			asm volatile("dmb" : : : "memory")
#define dsb()			asm volatile("dsb" : : : "memory")
#define isb()			asm volatile("isb")


/* incomplete types */
struct thread_t;


/* types */
// NOTE when changing thread_ctx_t also check if modifications
// 		to the interrupt service routine are required
typedef struct thread_ctx_t{
	struct thread_ctx_t *next;

	struct thread_ctx_t *this;
	uint32_t type;				/**< cf. thread_ctx_type_t */

	register_t control,
			   gpr_8_11[4],
			   gpr_4_7[4],
			   exc_return,
			   gpr_0_3[4],
			   gpr_12,
			   ret_lr;

	void *ret_addr;

	register_t xpsr;
} thread_ctx_t;


/* prototypes */
#ifdef BUILD_KERNEL
int av6m_core_init(uint32_t clock_mhz);
void av6m_core_sleep(void);
void av6m_core_panic(thread_ctx_t const *tc);

void av6m_nvic_init(void);
void av6m_nvic_int_enable(int_num_t num);
void av6m_nvic_int_disable(int_num_t num);
void av6m_nvic_int_prio_set(uint8_t num, uint8_t prio);

int av6m_systick_init(uint32_t clock_mhz);

bool av6m_int_enable(bool en);
bool av6m_int_enabled(void);

sc_t *av6m_sc_arg(struct thread_t *this_t);
void av6m_thread_ctx_init(thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg);
#endif // BUILD_KERNEL

int av6m_sc(sc_num_t num, void *param, size_t psize);

#endif // ARM_V6M_H
