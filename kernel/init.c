#include <arch/interrupt.h>
#include <arch/core.h>
#include <kernel/init.h>
#include <kernel/opt.h>
#include <kernel/kprintf.h>
#include <kernel/panic.h>
#include <kernel/stat.h>
#include <kernel/test.h>
#include <kernel/sched.h>
#include <sys/escape.h>
#include <sys/errno.h>
#include <version.h>


/* macros */
#define do_init_call(base, end, stage, pr_err)	{ if(errno == E_OK) _do_init_call(base, end, stage, pr_err); }


/* extern variables */
extern init_call_t __core_init0_base[],
				   __core_init1_base[],
				   __core_init2_base[],
				   __platform_init0_base[],
				   __platform_init1_base[],
				   __kernel_init0_base[],
				   __kernel_init1_base[],
				   __kernel_init2_base[],
				   __driver_init0_base[],
				   __driver_init1_base[],
				   __init_end[];


/* global variables */
kopt_t kopt = KOPT_INITIALISER();


/* local/static prototypes */
static void _do_init_call(init_call_t *base, init_call_t *end, char const *stage, bool p_err);


/* global functions */
void kernel(void){
	/* core local (stage: core 0) */
	do_init_call(__core_init0_base, __core_init1_base, "", false);

	if(PIR == 0){
		/* kernel basic services (stage: kernel 0) */
		do_init_call(__kernel_init0_base, __kernel_init1_base, "", false);

		/* platform basic and device specific (stage: platform 0, 1) */
		do_init_call(__platform_init0_base, __kernel_init0_base, "", false);

		/* print init message */
		kprintf(KMSG_ANY, "\n\t\t" FG_BLUE "::: boot system :::" RESET_ATTR "\n" VERSION "\n");

		/* kernel infrastructure (stage: kernel 1) */
		do_init_call(__kernel_init1_base, __kernel_init2_base, "kernel_init_stage1", true);

		/* kernel higher services (stage: kernel 2) */
		do_init_call(__kernel_init2_base, __driver_init0_base, "kernel_init_stage2", true);

		/* driver (stage: driver 0) */
		do_init_call(__driver_init0_base, __init_end, "driver_init_stage0", true);

#ifdef CONFIG_KERNEL_SMP
		/* core SMP (stage: core 1) */
		do_init_call(__core_init1_base, __platform_init0_base, "core_init_stage1", true);
#endif // CONFIG_KERNEL_SMP
	}

	/* stop execution if any of the init calls failed */
	if(errno != E_OK)
		kpanic(0x0, "error (%#x) during kernel init", errno);

	/* kernel statistics */
	kstat();

	/* kernel test */
	ktest();

	/* enable interrupts */
	int_enable(INT_ALL);

	/* kernel thread */
	while(1){
		// TODO implement kernel thread processing

		sched_yield(READY);
	}
}


/* local functions */
static void _do_init_call(init_call_t *base, init_call_t *end, char const *stage, bool p_err){
	init_call_t *p;


	for(p=base; p<end; p++){
		(void)(*p)();

#ifdef CONFIG_KERNEL_EARLY_PRINT
		if(errno != E_OK && p_err)
			WARN("\033[33minit-call %s at %#x failed with return code %#x\n\033[0m", stage, *p, errno);
#endif // CONFIG_KERNEL_EARLY_PRINT
	}
}
