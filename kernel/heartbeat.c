/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/timer.h>
#include <kernel/ktask.h>
#include <driver/gpio.h>
#include <sys/types.h>


/* types */
typedef struct{
	gpio_cfg_t port;

	uint16_t wave;
	uint16_t period_ms;
} dt_data_t;

typedef struct{
	gpio_t *gpio;
	dt_data_t *dtd;

	uint32_t time_ms;
	uint8_t idx;
} dev_data_t;


/* local/static prototypes */
static void task(void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_itf_t *dti = (gpio_itf_t*)dt_itf;
	gpio_t *gpio;
	dev_data_t heartbeat;


	gpio = gpio_create(dti, &dtd->port);

	if(gpio == 0x0)
		goto err_0;

	if(gpio_configure(gpio) != 0)
		goto err_1;

	heartbeat.gpio = gpio;
	heartbeat.dtd = dtd;
	heartbeat.time_ms = ktimer_ms();
	heartbeat.idx = 0;

	if(ktask_create(task, &heartbeat, sizeof(dev_data_t), 0x0, true) != 0)
		goto err_1;

	gpio_write(gpio, dtd->port.out_mask);

	return 0x0;


err_1:
	gpio_destroy(gpio);

err_0:
	return 0x0;
}

driver_probe("kernel,heartbeat", probe);

static void task(void *payload){
	dev_data_t *heartbeat = (dev_data_t*)payload;
	dt_data_t *dtd = heartbeat->dtd;
	gpio_t *gpio = heartbeat->gpio;
	intgpio_t v;
	uint32_t time_ms;


	time_ms = ktimer_ms();

	if(time_ms < heartbeat->time_ms + dtd->period_ms)
		return;

	v = gpio_read(gpio) | dtd->port.out_mask;

	if((dtd->wave & (0x1 << heartbeat->idx)) == 0)
		v ^= dtd->port.out_mask;

	gpio_write(gpio, v);

	heartbeat->time_ms = time_ms;
	heartbeat->idx = (heartbeat->idx + 1) & 0xf;
}
