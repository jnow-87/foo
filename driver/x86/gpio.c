/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/gpio.h>
#include <sys/gpio.h>



/* types */
typedef struct{
	char const *name;
	intgpio_t value;
} dev_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload);
static intgpio_t read(void *dt_data, void *payload);
static int write(intgpio_t v, void *dt_data, void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	gpio_ops_t ops;
	dev_data_t gpio;


	ops.configure = configure;
	ops.read = read;
	ops.write = write;

	gpio.name = name;
	gpio.value = 0x0;

	return gpio_itf_create(&ops, 0, dt_data, &gpio, sizeof(dev_data_t));
}

driver_probe("x86,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload){
	return 0;
}

static intgpio_t read(void *dt_data, void *payload){
	return ((dev_data_t*)payload)->value;
}

static int write(intgpio_t v, void *dt_data, void *payload){
	((dev_data_t*)payload)->value = v;

	return 0;
}
