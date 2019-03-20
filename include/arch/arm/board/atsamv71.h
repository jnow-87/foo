/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_ATSAMV71_H
#define ARCH_ATSAMV71_H


#include <sys/const.h>

#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

#include <arch/arm/board/types.h>
#include <arch/arm/board/atmel/pmc.h>
#include <arch/arm/register.h>

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* macros */
#define INT_VECTORS			16

#define INT_RESET			1
#define INT_NMI				2
#define INT_HARD_FAULT		3
#define INT_MEM_MGMT		4
#define INT_BUS_FAULT		5
#define INT_USAGE_FAULT		6
#define INT_SV_CALL			11
#define INT_DEBUG			12
#define INT_PEND_SV			14
#define INT_SYS_TICK		15

#define FLASH_SIZE			_2M
#define SRAM_SIZE			(_128k * 3)


/* prototypes */
#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

int atsamv71_init(void);

#endif // __x86_64__
#endif // _x86_
#endif // ASM


/* static variables */
#ifndef ASM
#ifndef _x86_
#ifndef __x86_64__

static arm_board_callbacks_t const arm_board_cbs = {
	.board_init = atsamv71_init,
	.peripheral_enable = pmc_per_enable,
	.peripheral_disable = pmc_per_disable,
};


#endif // __x86_64__
#endif // _x86_
#endif // ASM


#endif // ARCH_ATSAMV71_H
