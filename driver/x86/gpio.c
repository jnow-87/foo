/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <kernel/timer.h>
#include <driver/gpio.h>
#include <sys/gpio.h>


/* macros */
#define INT_MAGIC	0xfade


/* types */
typedef struct{
	uint8_t int_num;	/**< cf. int_num_t */
	uint16_t period_ms;
} dt_data_t;

typedef struct{
	char const *name;
	intgpio_t value;

	ktimer_t int_timer;
	bool int_timer_active;

	dt_data_t *dtd;
} dev_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload);
static intgpio_t read(void *dt_data, void *payload);
static int write(intgpio_t v, void *dt_data, void *payload);

static void int_hdlr(void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_ops_t ops;
	dev_data_t gpio;


	ops.configure = configure;
	ops.read = read;
	ops.write = write;

	gpio.name = name;
	gpio.value = 0x0;
	gpio.int_timer_active = false;
	gpio.dtd = dt_data;

	return gpio_itf_create(&ops, dtd->int_num, dtd, &gpio, sizeof(dev_data_t));
}

driver_probe("x86,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload){
	return 0;
}

static intgpio_t read(void *dt_data, void *payload){
	return ((dev_data_t*)payload)->value;
}

static int write(intgpio_t v, void *dt_data, void *payload){
	dev_data_t *gpio = (dev_data_t*)payload;


	gpio->value = v;

	if(v == INT_MAGIC){
		if(!gpio->int_timer_active)	ktimer_start(&gpio->int_timer, gpio->dtd->period_ms * 1000, int_hdlr, gpio, true);
		else						ktimer_abort(&gpio->int_timer);

		gpio->int_timer_active = !gpio->int_timer_active;
	}

	return 0;
}

static void int_hdlr(void *payload){
	dev_data_t *gpio = (dev_data_t*)payload;


	// update gpio value to allow the gpio layer
	// to detect a change and trigger an interrupt
	gpio->value++;
	int_foretell(gpio->dtd->int_num);
}
