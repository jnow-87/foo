#ifndef ARCH_CALLBACKS_H
#define ARCH_CALLBACKS_H


#ifndef ASM
#ifdef KERNEL

#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/page.h>
#include <kernel/ipi.h>
#include <kernel/interrupt.h>

#endif // KERNEL

#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>

#endif // ASM


/* types */
#ifdef KERNEL

typedef struct{
	/* virtual memory management */
	int (*page_entry_write)(page_t* const page);
	int (*page_entry_inval_idx)(unsigned int idx, bool sync_cores);
	int (*page_entry_inval_va)(void* const virt_addr, bool sync_cores);
	int (*page_entry_search)(page_t* const param, page_t* result);

	void (*copy_from_user)(void* const target, void* const src, unsigned int n, process_t* const this_p);
	void (*copy_to_user)(void* const target, void* const src, unsigned int n, process_t* const this_p);

	/* interrupts */
	void (*int_enable)(int_num_t num);
	void (*int_disable)(int_num_t num);
	int_num_t (*int_enabled)(void);
	int (*int_hdlr_register)(int_num_t num, int_hdlr_t hdlr);
	int (*int_hdlr_release)(int_num_t num);

	int (*ipi_sleep)(void);
	int (*ipi_wake)(ipi_t type, unsigned int core, bool bcast);

	/* threading */
	int (*thread_call)(thread_t* const this_t);
	void (*thread_kill)(int rcode);

	/* terminal I/O */
	int (*putchar)(int c);
	int (*puts)(const char* s);
	int (*getchar)(void);
} arch_callbacks_kernel_t;

#endif // KERNEL

typedef struct{
	/* time */
	timebase_t* (*timebase)(void);
	time_t* (*timebase_to_time)(timebase_t* const tb);

	/* atomics */
	int (*cas)(volatile int* const v, int old, int new);

	/* core */
	int (*core_id)(void);
	void (*core_sleep)(void);
	void (*core_halt)(void);

	/* syscall */
	int (*syscall)(syscall_t num, void* const param, unsigned int param_size);
} arch_callbacks_common_t;


#endif
