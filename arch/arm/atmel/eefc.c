/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/register.h>
#include <sys/errno.h>


/* macros */
#define EEFC_BASE	0x400e0c00

// EEFC registers
#define FMR			(EEFC_BASE | 0x00)

// EEFC bits
#define FMR_CLOE	26
#define FMR_SCOD	16
#define FMR_FWS		8
#define FMR_FRDY	0


/* global functions */
int eefc_init(void){
	// TODO
	// 	currently only the wait states are configured to ensure
	// 	the flash is working properly with different clock settings
	// 	however all other settings need to be checked if changes
	// 	are required
	mreg_w(FMR, (mreg_r(FMR) & ~(0xf << FMR_FWS))
			  | ((CONFIG_EEFC_WAITSTATE - 1) << FMR_FWS)
	);

	return E_OK;
}
