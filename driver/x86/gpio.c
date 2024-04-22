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

	gpio->name = name;
	gpio->value = 0x0;
	gpio->itf.configure = configure;
	gpio->itf.read = read;
	gpio->itf.write = write;
	gpio->itf.hw = gpio;

	return &gpio->itf;
}

driver_probe("x86,gpio", probe);

static int configure(gpio_cfg_t *cfg, void *hw){
	return 0;
}

static intgpio_t read(void *hw){
	return ((dev_data_t*)hw)->value;
}

static int write(intgpio_t v, void *hw){
	((dev_data_t*)hw)->value = v;

	return 0;
}
