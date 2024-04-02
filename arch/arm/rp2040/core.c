/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <arch/arm/v6m.h>
#include <kernel/init.h>
#include <sys/register.h>


/* macros */
#define SIO_CPUID		MREG(SIO_BASE | 0x000)


/* global functions */
int rp2040_core_id(void){
	return SIO_CPUID;
}


/* local functions */
static int init(void){
	return av6m_core_init(RP2040_SYSTEM_CLOCK_HZ / 1000000);
}

// cores have to be initialised after at least the clocks are setup and
// the rp2040 devtree platform configuration has been updated since the
// systick setup relies on those information
platform_init(1, all, init);
