/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <sys/compiler.h>
#include <sys/devtree.h>
#include <sys/errno.h>
#include <sys/math.h>
#include <sys/register.h>
#include <sys/types.h>


/* macros */
// registers
#define GPIO(x)					MREG(PADS_BANK0_BASE + 0x4 + x * 4)
#define BANK0_VOLTAGE_SELECT	MREG(PADS_BANK0_BASE + 0x0)
#define BANK1_VOLTAGE_SELECT	MREG(PADS_QSPI_BASE + 0x0)

#define GPIO_CTRL(x)			MREG(IO_BANK0_BASE + 0x4 + x * 8)
#define INTR(x)					MREG(IO_BANK0_BASE + 0x0f0 + x * 4)
#define INTE(core, x)			MREG(IO_BANK0_BASE + 0x100 + core * 0x30 + x * 4)

// register bits
#define GPIO_OD					7
#define GPIO_IE					6
#define GPIO_DRIVE				4
#define GPIO_PUE				3
#define GPIO_PDE				2
#define GPIO_SCHMITT			1
#define GPIO_SLEWFAST			0

#define GPIO_CTRL_FUNCSEL		0

#define INTE_EDGE_LOW			2
#define INTE_EDGE_HIGH			3

// pad configurations
#define RESET_PAD_DRIVE			RP2040_PAD_DRV_4MA
#define RESET_PAD_FLAGS			(RP2040_PAD_FLAG_OUTPUT_EN | RP2040_PAD_FLAG_INPUT_EN | RP2040_PAD_FLAG_PULLDOWN_EN | RP2040_PAD_FLAG_SCHMITT_EN)


/* static variables */
// default pad configuration per function, cf. rp2040_pad_func_t
static rp2040_pad_cfg_t pad_func_cfgs[] = {
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// xip
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// spi
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// uart
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// i2c
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// pwm
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// sio
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// pio0
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// pio1
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// clk
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// usb
	{ .flags = RESET_PAD_FLAGS,	.drive = RESET_PAD_DRIVE, },	// reset
};


/* global functions */
void rp2040_pads_init(void){
	rp2040_platform_cfg_t const *plt;


	plt = devtree_arch_payload("rp2040,platform");

	rp2040_resets_release((0x1 << RP2040_RST_PADS_BANK0) | (0x1 << RP2040_RST_IO_BANK0));

	for(uint8_t i=0; i<30; i++){
		GPIO_CTRL(i) = plt->gpio_funcsel[i] << GPIO_CTRL_FUNCSEL;
		rp2040_pad_init(i, &pad_func_cfgs[MIN(plt->gpio_funcsel[i], sizeof_array(pad_func_cfgs) - 1)]);
	}

	BANK0_VOLTAGE_SELECT = plt->gpio_v33 ? 0 : 1;
	BANK1_VOLTAGE_SELECT = plt->gpio_v33 ? 0 : 1;
}

int rp2040_pad_init(uint8_t pad, rp2040_pad_cfg_t *cfg){
	uint8_t int_reg;
	uint32_t int_mask;


	if(pad > 29)
		return_errno(E_LIMIT);

	// configure pad
	GPIO(pad) = ((!((bool)(cfg->flags & RP2040_PAD_FLAG_OUTPUT_EN))) << GPIO_OD)
			  | (((bool)(cfg->flags & RP2040_PAD_FLAG_INPUT_EN)) << GPIO_IE)
			  | (((bool)(cfg->flags & RP2040_PAD_FLAG_PULLUP_EN)) << GPIO_PUE)
			  | (((bool)(cfg->flags & RP2040_PAD_FLAG_PULLDOWN_EN)) << GPIO_PDE)
			  | (((bool)(cfg->flags & RP2040_PAD_FLAG_SCHMITT_EN)) << GPIO_SCHMITT)
			  | (((bool)(cfg->flags & RP2040_PAD_FLAG_SLEWFAST)) << GPIO_SLEWFAST)
			  | (cfg->drive << GPIO_DRIVE)
			  ;

	// configure interrupt
	int_reg = pad / 8;

	// level interrupts are not supported since they would overload
	// the system if the trigger level is active
	int_mask = (cfg->flags & RP2040_PAD_FLAG_INT_EN) ? ((0x1 << INTE_EDGE_HIGH) | (0x1 << INTE_EDGE_LOW)) : 0x0;
	int_mask <<= (pad - int_reg * 8) * 4;

	INTR(int_reg) |= int_mask; // clear stale events
	INTE(0, int_reg) |= int_mask;

	return 0;
}
