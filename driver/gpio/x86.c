/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <driver/gpio.h>



/* types */
typedef struct{
	char const *name;
	gpio_type_t value;
} dev_data_t;


/* local/static prototypes */
static gpio_type_t read(void *hw);
static int write(gpio_type_t v, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *gpio;
	gpio_ops_t ops;


	gpio = kmalloc(sizeof(dev_data_t));

	if(gpio == 0x0)
		goto err_0;

	gpio->name = name;
	gpio->value = 0x0;

	ops.write = write;
	ops.read = read;

	if(gpio_create(name, &ops, dt_data, gpio) == 0x0)
		goto err_1;

	return 0x0;


err_1:
	kfree(gpio);

err_0:
	return 0x0;
}

driver_probe("x86,gpio", probe);

static gpio_type_t read(void *hw){
	dev_data_t *gpio;


	gpio = (dev_data_t*)hw;
	DEBUG("%s: read %#x\n", gpio->name, gpio->value);

	return gpio->value;
}

static int write(gpio_type_t v, void *hw){
	dev_data_t *gpio;


	gpio = (dev_data_t*)hw;
	gpio->value = v;

	DEBUG("%s: write %#x\n", gpio->name, gpio->value);

	return E_OK;
}
