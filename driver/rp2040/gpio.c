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
// registers
#define GPIO(x)			MREG(PADS_BANK0_BASE + 0x4 + x * 4)

#define GPIO_IN			MREG(SIO_BASE + 0x4)
#define GPIO_OUT		MREG(SIO_BASE + 0x10)
#define GPIO_OE			MREG(SIO_BASE + 0x20)
#define INTR(x)			MREG(IO_BANK0_BASE + 0x0f0 + x * 4)
#define INTE(core, x)	MREG(IO_BANK0_BASE + 0x100 + core * 0x30 + x * 4)

// register bits
#define GPIO_OD			7
#define GPIO_IE			6
#define GPIO_DRIVE		4
#define GPIO_PUE		3
#define GPIO_PDE		2
#define GPIO_SCHMITT	1
#define GPIO_SLEWFAST	0

#define INTE_EDGE_LOW	2
#define INTE_EDGE_HIGH	3


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

	uint8_t int_num;	/**< cf. int_num_t */
} dt_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload);
static intgpio_t read(void *dt_data, void *payload);
static int write(intgpio_t v, void *dt_data, void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_ops_t ops;
	gpio_itf_t *itf;


	ops.configure = configure;
	ops.read = read;
	ops.write = write;

	itf = gpio_itf_create(&ops, dtd->int_num, dtd, &(uint32_t){ 0 }, sizeof(uint32_t));

	if(itf != 0x0 && dtd->int_num)
		av6m_nvic_int_enable(dtd->int_num);

	return itf;
}

driver_probe("rp2040,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload){
	uint8_t drive[] = { 0, 1, 2, 0, 3 };
	dt_data_t *dtd = (dt_data_t*)dt_data;
	uint32_t pin;
	uint8_t drive_idx;
	uint8_t int_reg;
	uint32_t int_mask;


	for(uint8_t i=0; i<32; i++){
		pin = 0x1 << i;

		if(((cfg->in_mask | cfg->out_mask)  & pin) == 0)
			continue;

		/* configure pad */
		drive_idx = (((bool)(dtd->drive_4ma & pin)))
				  | (((bool)(dtd->drive_8ma & pin)) << 0x1)
				  | (((bool)(dtd->drive_12ma & pin)) << 0x2)
				  ;

		GPIO(i) = ((!((bool)(cfg->out_mask & pin))) << GPIO_OD)
				| (((bool)(cfg->in_mask & pin)) << GPIO_IE)
				| (((bool)(dtd->pullup_mask & pin)) << GPIO_PUE)
				| (((bool)(dtd->pulldown_mask & pin)) << GPIO_PDE)
				| (((bool)(dtd->schmitt_en & pin)) << GPIO_SCHMITT)
				| (((bool)(dtd->slewfast & pin)) << GPIO_SLEWFAST)
				| (drive[drive_idx] << GPIO_DRIVE)
				;

		/* configure interrupt */
		int_reg = i / 8;

		// level interrupts are not supported since they would overload
		// the system if the trigger level is active
		int_mask = (bool)(cfg->int_mask & pin);
		int_mask = (int_mask << INTE_EDGE_HIGH) | (int_mask << INTE_EDGE_LOW);
		int_mask <<= (i - int_reg * 8) * 4;

		INTR(int_reg) |= int_mask; // clear stale events
		INTE(0, int_reg) |= int_mask;
	}

	GPIO_OE = (GPIO_OE | cfg->out_mask) & ~cfg->in_mask;

	return 0;
}

static intgpio_t read(void *dt_data, void *payload){
	uint32_t *out_last = (uint32_t*)payload;


	INTR(0) = INTR(0);	// clear interrupts

	// GPIO_IN does not reflect the state of output pins, hence if the same
	// gpio interface is used by multiple drivers they would overwrite each
	// other, since the pin state is read before writing a new value. To
	// prevent that, out_last is used to capture the last value written to
	// output pins and ored to the read value to be compatible with the gpio
	// driver layer.
	return GPIO_IN | *out_last;
}

static int write(intgpio_t v, void *dt_data, void *payload){
	uint32_t *out_last = (uint32_t*)payload;


	*out_last = v & GPIO_OE;
	GPIO_OUT = v;

	return 0;
}
