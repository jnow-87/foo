/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/v6m.h>
#include <sys/types.h>


/* macros */
// register bits
#define PRIMASK_PM	0


/* global functions */
bool av6m_int_enable(bool en){
	bool s;


	s = av6m_int_enabled();

	msr(primask, (!en << PRIMASK_PM));
	isb();

	return s;
}

bool av6m_int_enabled(void){
	return ((mrs(primask) & (0x1 << PRIMASK_PM)) == 0);
}
