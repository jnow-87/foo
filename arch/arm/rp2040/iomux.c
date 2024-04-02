/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <sys/devtree.h>
#include <sys/register.h>
#include <sys/types.h>


/* macros */
#define BANK0_VOLTAGE_SELECT	MREG(PADS_BANK0_BASE + 0x0)
#define BANK1_VOLTAGE_SELECT	MREG(PADS_QSPI_BASE + 0x0)

#define GPIO_CTRL(x)			MREG(IO_BANK0_BASE + 0x4 + x * 8)
#define GPIO_CTRL_FUNCSEL		0


/* global functions */
void rp2040_iomux_init(void){
	rp2040_platform_cfg_t const *plt;


	plt = devtree_arch_payload("rp2040,platform");

	rp2040_resets_release((0x1 << RP2040_RST_PADS_BANK0) | (0x1 << RP2040_RST_IO_BANK0));

	for(uint8_t i=0; i<30; i++){
		GPIO_CTRL(i) = plt->gpio_funcsel[i] << GPIO_CTRL_FUNCSEL;
	}

	BANK0_VOLTAGE_SELECT = plt->gpio_v33 ? 0 : 1;
	BANK1_VOLTAGE_SELECT = plt->gpio_v33 ? 0 : 1;
}
