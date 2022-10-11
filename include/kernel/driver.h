/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_DRIVER_H
#define KERNEL_DRIVER_H


#include <sys/compiler.h>


/* macros */
#define driver_probe(_compatible, _probe) \
	static char const driver_comp_##_probe[] __used = _compatible; \
	static driver_t driver_##_probe __linker_array(".driver") = { \
		.compatible = driver_comp_##_probe, \
		.probe = _probe, \
	}


/* types */
typedef struct{
	char const *compatible;
	void * (*probe)(char const *name, void *dt_data, void *dt_itf);
} driver_t;


/* prototypes */
int driver_load(void);


#endif // KERNEL_DRIVER_H
