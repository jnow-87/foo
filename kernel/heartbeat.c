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
	uint16_t active_ms,
			 inactive_ms;
} dt_data_t;

typedef struct{
	gpio_t *gpio;

	uint32_t time_ms;
	uint16_t periods[2];
	uint8_t state;
} dev_data_t;


/* local/static prototypes */
static void task(void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_t *dti = (gpio_t*)dt_itf;
	dev_data_t heartbeat;


	heartbeat.gpio = dti;
	heartbeat.time_ms = ktimer_ms();
	heartbeat.periods[0] = dtd->active_ms;
	heartbeat.periods[1] = dtd->inactive_ms;
	heartbeat.state = 0;

	if(ktask_create(task, &heartbeat, sizeof(dev_data_t), 0x0, true) != 0)
		return 0x0;

	gpio_write(dti, dti->cfg->out_mask);

	return 0x0;
}

driver_probe("kernel,heartbeat", probe);

#include <kernel/kprintf.h>

static void task(void *payload){
	dev_data_t *heartbeat = (dev_data_t*)payload;
	gpio_t *gpio = heartbeat->gpio;
	uint32_t time_ms;


	time_ms = ktimer_ms();

	if(time_ms < heartbeat->time_ms + heartbeat->periods[heartbeat->state])
		return;

	gpio_write(gpio, gpio_read(gpio) ^ gpio->cfg->out_mask);

	heartbeat->time_ms = time_ms;
	heartbeat->state = (heartbeat->state + 1) & 0x1;
}
