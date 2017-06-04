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


/* types */
#ifdef KERNEL

typedef struct{
	/* virtual memory management */
	errno_t (*page_entry_write)(page_t const *page);
	errno_t (*page_entry_inval_idx)(unsigned int idx, bool sync_cores);
	errno_t (*page_entry_inval_va)(void *virt_addr, bool sync_cores);
	errno_t (*page_entry_search)(page_t const *param, page_t *result);

	errno_t (*copy_from_user)(void *target, void const *src, unsigned int n, process_t const *this_p);
	errno_t (*copy_to_user)(void *target, void const *src, unsigned int n, process_t const *this_p);

	/* interrupts */
	errno_t (*int_enable)(int_type_t mask);
	int_type_t (*int_enabled)(void);
	errno_t (*int_hdlr_register)(int_num_t num, int_hdlr_t hdlr);
	errno_t (*int_hdlr_release)(int_num_t num);

	errno_t (*ipi_sleep)(void);
	errno_t (*ipi_wake)(ipi_t type, unsigned int core, bool bcast);

	/* threading */
	errno_t (*thread_call)(thread_t *this_t);
	errno_t (*thread_kill)(int rcode);

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
	errno_t (*cas)(volatile int *v, int old, int new);

	/* core */
	int (*core_id)(void);
	void (*core_sleep)(void);
	void (*core_halt)(void);

	/* syscall */
	errno_t (*syscall)(syscall_t num, void *param, unsigned int param_size);

	/* main entry */
	int (*libmain)(int argc, char **argv);
} arch_callbacks_common_t;


#endif
