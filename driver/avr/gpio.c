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
	uint8_t int_num;	/**< cf. int_num_t */
} dt_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload);
static intgpio_t read(void *dt_data, void *payload);
static int write(intgpio_t v, void *dt_data, void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_itf_t *itf;
	gpio_ops_t ops;


	ops.configure = configure;
	ops.read = read;
	ops.write = write;

	itf = gpio_itf_create(&ops, dtd->int_num, dtd, 0x0, 0);

	if(itf != 0x0 && dtd->int_num)
		*dtd->pcicr = bits_set(*dtd->pcicr, 0x1 << (dtd->int_num - INT_PCINT0));

	return itf;
}

driver_probe("avr,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_regs_t *regs = dtd->regs;


	// configure port
	regs->ddr = bits_set(regs->ddr, cfg->out_mask);
	regs->port = bits_set(regs->port, dtd->pullup_mask);

	// configure interrupts
	*dtd->pcmsk = bits_set(*dtd->pcmsk, cfg->int_mask);

	return 0;
}

static intgpio_t read(void *dt_data, void *payload){
	return ((dt_data_t*)dt_data)->regs->pin;
}

static int write(intgpio_t v, void *dt_data, void *payload){
	((dt_data_t*)dt_data)->regs->port = v;

	return 0;
}
