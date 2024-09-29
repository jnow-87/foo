/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <kernel/timer.h>
#include <driver/gpio.h>
#include <sys/gpio.h>
#include <sys/types.h>


/* macros */
#define INT_MAGIC	0xfade


/* types */
typedef struct{
	uint16_t period_ms;
} dt_data_t;

typedef struct{
	char const *name;
	intgpio_t value;

	int_num_t int_num;
	ktimer_t int_timer;
	bool int_timer_active;

	dt_data_t *dtd;
	gpio_itf_t itf;
} dev_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *hw);
static intgpio_t read(void *hw);
static int write(intgpio_t v, void *hw);

static void timer_hdlr(void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *gpio;


	gpio = kmalloc(sizeof(dev_data_t));

	if(gpio == 0x0)
		return 0x0;

	gpio->name = name;
	gpio->value = 0x0;
	gpio->int_num = 0;
	gpio->int_timer_active = false;
	gpio->dtd = dt_data;

	gpio->itf.configure = configure;
	gpio->itf.read = read;
	gpio->itf.write = write;

	gpio->itf.hw = gpio;

	return &gpio->itf;
}

driver_probe("x86,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *hw){
	((dev_data_t*)hw)->int_num = cfg->int_num;

	return 0;
}

static intgpio_t read(void *hw){
	return ((dev_data_t*)hw)->value;
}

static int write(intgpio_t v, void *hw){
	dev_data_t *gpio = (dev_data_t*)hw;


	gpio->value = v;

	if(v == INT_MAGIC){
		if(!gpio->int_timer_active)	ktimer_start(&gpio->int_timer, gpio->dtd->period_ms * 1000, timer_hdlr, gpio, true);
		else						ktimer_abort(&gpio->int_timer);

		gpio->int_timer_active = !gpio->int_timer_active;
	}

	return 0;
}

static void timer_hdlr(void *payload){
	dev_data_t *gpio = (dev_data_t*)payload;


	// update gpio value to allow the gpio layer
	// to detect a change and trigger an interrupt
	gpio->value++;
	int_foretell(gpio->int_num);
}
