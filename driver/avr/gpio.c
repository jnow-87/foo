/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/driver.h>
#include <driver/gpio.h>
#include <sys/types.h>
#include <sys/gpio.h>


/* types */
typedef struct{
	uint8_t volatile pin,
					 ddr,
					 port;
} gpio_regs_t;

typedef struct{
	// port registers
	gpio_regs_t *regs;

	// interrupt registers
	uint8_t volatile *pcicr,
					 *pcmsk;

	// configuration
	gpio_cfg_t cfg;
} dt_data_t;


/* local/static prototypes */
static gpio_int_t read(void *hw);
static int write(gpio_int_t v, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_t *gpio;
	gpio_regs_t *regs = dtd->regs;
	gpio_cfg_t *cfg = &dtd->cfg;
	gpio_ops_t ops;


	/* create device */
	ops.read = read;
	ops.write = write;

	gpio = gpio_create(&ops, cfg, dtd);

	if(gpio == 0x0)
		return 0x0;

	/* configure */
	// port
	regs->port |= cfg->in_mask | cfg->out_mask | cfg->int_mask;
	regs->ddr |= cfg->out_mask | cfg->int_mask;

	// interrupts
	*dtd->pcmsk |= cfg->int_mask;

	if(cfg->int_num)
		*dtd->pcicr |= (0x1 << (cfg->int_num - INT_PCINT0));

	return gpio;
}

driver_probe("avr,gpio", probe);

static gpio_int_t read(void *hw){
	return ((dt_data_t*)hw)->regs->pin;
}

static int write(gpio_int_t v, void *hw){
	((dt_data_t*)hw)->regs->port = v;

	return 0;
}
