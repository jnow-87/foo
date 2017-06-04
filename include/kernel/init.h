#ifndef KERNEL_INIT_H
#define KERNEL_INIT_H


#include <sys/compiler.h>
#include <sys/errno.h>


/* types */
typedef errno_t (*init_call_t)(void);


/* macros */
#define init_call(call, level, stage)	static init_call_t init_call_##call __section("."#level"_init_stage"#stage) __used = call;

/**
 * currently defined init levels, order according to calling sequence
 *
 *	core 0		-	core local initialisation, e.g. caches, mmu
 * 	kernel 0	-	kernel basic services, e.g. kmalloc
 *	platform 0	-	platform basic initialisation
 *	platform 1	-	platform device initialisation (afterwards uart shall be usable)
 *	kernel 1	-	kernel infrastructure, e.g. umalloc, basic filesystem
 *	kernel 2	-	kernel higher services, e.g. rootfs, devfs, scheduler
 *	driver		-	driver
 *	core 1		-	core SMP initialisation
 */
#define core_init(stage, call)			init_call(call, core, stage)
#define platform_init(stage, call)		init_call(call, platform, stage)
#define kernel_init(stage, call)		init_call(call, kernel, stage)
#define driver_init(call)				init_call(call, driver, 0)


#endif // KERNEL_INIT_H
