#ifndef ARCH_CALLBACKS_H
#define ARCH_CALLBACKS_H


#ifndef ASM
#ifdef KERNEL

#include <arch/interrupt.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/page.h>
#include <kernel/ipi.h>

#endif // KERNEL

#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>

#endif // ASM


/* incomplete types */
#ifdef KERNEL

struct process_t;
struct thread_t;

#endif // KERNEL


/* types */
#ifdef KERNEL

typedef struct{
	/* virtual memory management */
	int (*page_entry_write)(page_t const *page);
	int (*page_entry_inval_idx)(unsigned int idx, bool sync_cores);
	int (*page_entry_inval_va)(void *virt_addr, bool sync_cores);
	int (*page_entry_search)(page_t const *param, page_t *result);

	int (*copy_from_user)(void *target, void const *src, unsigned int n, struct process_t const *this_p);
	int (*copy_to_user)(void *target, void const *src, unsigned int n, struct process_t const *this_p);

	/* interrupts */
	int (*int_enable)(int_type_t mask);
	int_type_t (*int_enabled)(void);
	int (*int_hdlr_register)(int_num_t num, int_hdlr_t hdlr);
	int (*int_hdlr_release)(int_num_t num);

	int (*ipi_sleep)(void);
	int (*ipi_wake)(ipi_t type, unsigned int core, bool bcast);

	/* threading */
	thread_context_t * (*thread_context_init)(struct thread_t *this_t, void *thread_arg);

	/* terminal I/O */
	char (*putchar)(char c);
	int (*puts)(char const *s);
} arch_callbacks_kernel_t;

#endif // KERNEL

typedef struct{
	/* time */
	timebase_t * (*timebase)(void);
	time_t * (*timebase_to_time)(timebase_t *tb);

	/* atomics */
	int (*cas)(volatile int *v, int old, int new);

	/* core */
	int (*core_id)(void);
	void (*core_sleep)(void);
	void (*core_halt)(void);

	/* syscall */
	int (*sc)(sc_t num, void *param, size_t psize);

	/* libsys functionality */
	int (*lib_init)(void);
	int (*lib_main)(int argc, char **argv);
} arch_callbacks_common_t;

typedef struct{
	uint16_t core_clock_khz;
	uint16_t timebase_clock_khz;
} arch_info_t;


#endif
