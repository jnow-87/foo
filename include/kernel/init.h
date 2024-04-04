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
#include <sys/devicetree.h>
#include <sys/errno.h>


/* types */
typedef struct{
	int (*call)(void);

#ifdef CONFIG_KERNEL_INIT_DEBUG
	char const *name;
#endif // CONFIG_KERNEL_INIT_DEBUG
} init_call_t;


/* macros */
#ifdef CONFIG_KERNEL_INIT_DEBUG
# define init_call(_call, level, stage, cores) \
	static init_call_t init_call_##_call \
		__linker_array("."#level"_init_stage"#stage"_"#cores) = { \
			.call = _call, \
			.name = __FILE__ ":" #_call, \
		}

#else
# define init_call(_call, level, stage, cores) \
	static init_call_t init_call_##_call \
		__linker_array("."#level"_init_stage"#stage"_"#cores) = { \
			.call = _call, \
		}
#endif // CONFIG_KERNEL_INIT_DEBUG

#define discard_call(call) \
	static int call(void) __unused __section(".kernel_discard")

/**
 * currently defined init levels, order according to calling sequence
 *
 * 	platform 0	-	platform 1st stage initialisation
 * 	platform 1	-	platform 2nd stage initialisation
 * 	kernel 0	-	kernel basic services, e.g. kmalloc, umalloc
 * 	kernel 1	-	kernel infrastructure, e.g. rootfs
 * 	kernel 2	-	kernel higher services, e.g. devfs, scheduler
 */
#define platform_init(stage, cores, call)	init_call(call, platform, stage, cores_##cores)
#define kernel_init(stage, call)			init_call(call, kernel, stage, first)


#endif // KERNEL_INIT_H
