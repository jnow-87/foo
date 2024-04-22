/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/gpio.h>
#include <sys/gpio.h>
#include <sys/register.h>
#include <sys/types.h>


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

	uint8_t pullup_mask;
} dt_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *hw);
static intgpio_t read(void *hw);
static int write(intgpio_t v, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	gpio_itf_t *itf;


	itf = kmalloc(sizeof(gpio_itf_t));

	if(itf == 0x0)
		return 0x0;

	itf->configure = configure;
	itf->read = read;
	itf->write = write;
	itf->hw = dt_data;

	return itf;
}

driver_probe("avr,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;
	gpio_regs_t *regs = dtd->regs;


	// configure port
	regs->ddr = bits_set(regs->ddr, cfg->out_mask);
	regs->port = bits_set(regs->port, dtd->pullup_mask);

	// configure interrupts
	*dtd->pcmsk = bits_set(*dtd->pcmsk, cfg->int_mask);

	if(cfg->int_num)
		*dtd->pcicr = bits_set(*dtd->pcicr, 0x1 << (cfg->int_num - INT_PCINT0));

	return 0;
}

static intgpio_t read(void *hw){
	return ((dt_data_t*)hw)->regs->pin;
}

static int write(intgpio_t v, void *hw){
	((dt_data_t*)hw)->regs->port = v;

	return 0;
}
