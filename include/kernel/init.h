/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_INIT_H
#define KERNEL_INIT_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/errno.h>


/* types */
typedef struct{
	int (*call)(void);

#ifdef CONFIG_KERNEL_LOG_INIT
	char const *name;
#endif // CONFIG_KERNEL_LOG_INIT
} init_call_t;


/* macros */
#ifdef CONFIG_KERNEL_LOG_INIT
# define init_call(_call, level, stage) \
	static init_call_t init_call_##_call __linker_array("."#level"_init_stage"#stage) = { \
		.call = _call, \
		.name = __FILE__ ":" #_call, \
	}

#else
# define init_call(_call, level, stage) \
	static init_call_t init_call_##_call __linker_array("."#level"_init_stage"#stage) = { \
		.call = _call, \
	}
#endif // CONFIG_KERNEL_LOG_INIT

/**
 * currently defined init levels, order according to calling sequence
 *
 * 	core		-	core local initialisation, e.g. caches, mmu
 * 	platform 0	-	platform basic initialisation
 * 	platform 1	-	platform device initialisation
 * 	kernel 0	-	kernel basic services, e.g. kmalloc, umalloc
 * 	kernel 1	-	kernel infrastructure, e.g. rootfs
 * 	kernel 2	-	kernel higher services, e.g. devfs, scheduler
 */
#define core_init(call)					init_call(call, core, 0)
#define platform_init(stage, call)		init_call(call, platform, stage)
#define kernel_init(stage, call)		init_call(call, kernel, stage)


#endif // KERNEL_INIT_H
