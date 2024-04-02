/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <sys/register.h>
#include <sys/types.h>


/* macros */
#define RESET			MREG(RESETS_BASE + 0x0)
#define RESET_DONE		MREG(RESETS_BASE + 0x8)


/* global functions */
void rp2040_resets_release(uint32_t mask){
	RESET &= ~mask;

	while((RESET_DONE & mask) != mask);
}

void rp2040_resets_halt(uint32_t mask){
	RESET |= mask & 0x00ffffff;
}
