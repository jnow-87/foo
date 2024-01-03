/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <sys/types.h>
#include <sys/register.h>
#include <sys/compiler.h>
#include <sys/devicetree.h>
#include <sys/errno.h>


/* macros */
#define SYSTCSR		MREG(AV6M_PPB_BASE + 0xe010)
#define SYSTRVR		MREG(AV6M_PPB_BASE + 0xe014)
#define SYSTCVR		MREG(AV6M_PPB_BASE + 0xe018)
#define SYSTCALIB	MREG(AV6M_PPB_BASE + 0xe01c)

#define CSR_COUNTFLAG	16
#define CSR_CLKSOURCE	2
#define CSR_TICKINT		1
#define CSR_ENABLE		0

#define CALIB_NOREF		31
#define CALIB_SKEW		30
#define CALIB_TENMS		0


/* global functions */
int av6m_systick_init(uint32_t clock_mhz){
	register_t cnt = clock_mhz * DEVTREE_ARCH_TIMER_CYCLE_TIME_US;


	if(cnt == 0 || (cnt & 0xff000000) != 0)
		return_errno(E_LIMIT);

	SYSTRVR = cnt;
	SYSTCSR = (0x1 << CSR_CLKSOURCE) | (0x1 << CSR_TICKINT) | (0x1 << CSR_ENABLE);

	return 0;
}
