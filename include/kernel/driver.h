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
#define interface_probe(_compatible, _probe) \
	static char const driver_comp_##_probe[] __used = _compatible; \
	static interface_driver_t driver_##_probe __section(".interface_driver") __used = { \
		.compatible = driver_comp_##_probe, \
		.probe = _probe, \
	}

#define device_probe(_compatible, _probe) \
	static char const driver_comp_##_probe[] __used = _compatible; \
	static device_driver_t driver_##_probe __section(".device_driver") __used = { \
		.compatible = driver_comp_##_probe, \
		.probe = _probe, \
	}


/* types */
typedef struct{
	char const *compatible;
	void * (*probe)(void *data, void *itf);
} interface_driver_t;

typedef struct{
	char const *compatible;
	int (*probe)(char const *name, void *data, void *itf);
} device_driver_t;


/* prototypes */
void driver_load(void);


#endif // KERNEL_DRIVER_H
