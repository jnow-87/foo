/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <arch/arm/v6m.h>
#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/gpio.h>
#include <sys/gpio.h>
#include <sys/register.h>
#include <sys/types.h>


/* macros */
#define INTR(x)		MREG(IO_BANK0_BASE + 0x0f0 + x * 4)

#define GPIO_IN		MREG(SIO_BASE + 0x4)
#define GPIO_OUT	MREG(SIO_BASE + 0x10)
#define GPIO_OE		MREG(SIO_BASE + 0x20)


/* types */
typedef struct{
	uint32_t pullup_mask,
			 pulldown_mask,
			 schmitt_en,
			 slewfast,
			 drive_2ma,
			 drive_4ma,
			 drive_8ma,
			 drive_12ma;
} dt_data_t;

typedef struct{
	uint32_t out_last;

	dt_data_t *dtd;
	gpio_itf_t itf;
} dev_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *hw);
static intgpio_t read(void *hw);
static int write(intgpio_t v, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *gpio;


	gpio = kmalloc(sizeof(dev_data_t));

	if(gpio == 0x0)
		return 0x0;

	gpio->out_last = 0;
	gpio->dtd = dt_data;

	gpio->itf.configure = configure;
	gpio->itf.read = read;
	gpio->itf.write = write;

	gpio->itf.hw = gpio;

	return &gpio->itf;
}

driver_probe("rp2040,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *hw){
	uint8_t drive[] = {
		RP2040_PAD_DRV_2MA,
		RP2040_PAD_DRV_4MA,
		RP2040_PAD_DRV_8MA,
		RP2040_PAD_DRV_2MA,
		RP2040_PAD_DRV_12MA
	};
	dt_data_t *dtd = ((dev_data_t*)hw)->dtd;
	uint32_t mask;
	uint8_t drive_idx;
	rp2040_pad_cfg_t pad;


	for(uint8_t i=0; i<32; i++){
		mask = 0x1 << i;

		if(((cfg->in_mask | cfg->out_mask) & mask) == 0)
			continue;

		/* configure pad */
		drive_idx = (((bool)(dtd->drive_4ma & mask)))
				  | (((bool)(dtd->drive_8ma & mask)) << 0x1)
				  | (((bool)(dtd->drive_12ma & mask)) << 0x2)
				  ;

		pad.drive = drive[drive_idx];
		pad.flags = ((cfg->in_mask & mask) ? RP2040_PAD_FLAG_INPUT_EN : 0x0)
				  | ((cfg->out_mask & mask) ? RP2040_PAD_FLAG_OUTPUT_EN : 0x0)
				  | ((cfg->int_mask & mask) ? RP2040_PAD_FLAG_INT_EN : 0x0)
				  | ((dtd->pullup_mask & mask) ? RP2040_PAD_FLAG_PULLUP_EN : 0x0)
				  | ((dtd->pulldown_mask & mask) ? RP2040_PAD_FLAG_PULLDOWN_EN : 0x0)
				  | ((dtd->schmitt_en & mask) ? RP2040_PAD_FLAG_SCHMITT_EN : 0x0)
				  | ((dtd->slewfast & mask) ? RP2040_PAD_FLAG_SLEWFAST : 0x0)
				  ;

		if(rp2040_pad_init(i, &pad) != 0)
			return -errno;
	}

	GPIO_OE = (GPIO_OE | cfg->out_mask) & ~cfg->in_mask;

	if(cfg->int_num)
		av6m_nvic_int_enable(cfg->int_num);

	return 0;
}

static intgpio_t read(void *hw){
	dev_data_t *gpio = (dev_data_t*)hw;


	INTR(0) = INTR(0);	// clear interrupts

	// GPIO_IN does not reflect the state of output pins, hence if the same
	// gpio interface is used by multiple drivers they would overwrite each
	// other, since the pin state is read before writing a new value. To
	// prevent that, out_last is used to capture the last value written to
	// output pins and ored to the read value to be compatible with the gpio
	// driver layer.
	return GPIO_IN | gpio->out_last;
}

static int write(intgpio_t v, void *hw){
	dev_data_t *gpio = (dev_data_t*)hw;


	gpio->out_last = v & GPIO_OE;
	GPIO_OUT = v;

	return 0;
}
