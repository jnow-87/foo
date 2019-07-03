/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef MEMLAYOUT_CONFIG_H
#define MEMLAYOUT_CONFIG_H


#include <config/config.h>


/* static variables */
static char const *required_nodes[] = {
	"kernel_stack",
	"kernel_heap",
	"process_heap",

#ifdef CONFIG_AVR
	"kernel_flash",
	"process_flash",
#endif // CONFIG_AVR

#ifdef CONFIG_POWERPC_QORIQ
	"ccsr",
	"dcsr",
	"dpaa",
#endif // CONFIG_POWERPC_QORIQ
};



#endif // MEMLAYOUT_CONFIG_H
