/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_ATSAMV71_H
#define ARCH_ATSAMV71_H


#include <sys/const.h>


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


#endif // ARCH_ATSAMV71_H
