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
#include <sys/errno.h>
#include <sys/gpio.h>
#include <sys/types.h>


/* types */
typedef struct{
	uint8_t pin;

	uint16_t wave;
	uint16_t period_ms;
} dt_data_t;

typedef struct{
	dt_data_t *dtd;
	gpio_t *dti;

	uint32_t time_ms;
	uint8_t idx;
} dev_data_t;


/* local/static prototypes */
static void task(void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_t *dti = (gpio_t*)dt_itf;
	intgpio_t mask = (0x1 << dtd->pin);
	dev_data_t heartbeat;


	if((mask & dti->cfg->out_mask) != mask)
		goto_errno(err, E_INVAL);

	heartbeat.dtd = dtd;
	heartbeat.dti = dti;
	heartbeat.time_ms = ktimer_ms();
	heartbeat.idx = 0;

	if(ktask_create(task, &heartbeat, sizeof(dev_data_t), 0x0, true) != 0)
		goto err;

	gpio_write(dti, mask, mask);

	return 0x0;


err:
	return 0x0;
}

driver_probe("kernel,heartbeat", probe);

static void task(void *payload){
	dev_data_t *heartbeat = (dev_data_t*)payload;
	dt_data_t *dtd = heartbeat->dtd;
	gpio_t *dti = heartbeat->dti;
	intgpio_t mask = (0x1 << dtd->pin);
	intgpio_t v;
	uint32_t time_ms;


	time_ms = ktimer_ms();

	if(time_ms < heartbeat->time_ms + dtd->period_ms)
		return;

	v = gpio_read(dti, mask) | mask;

	if((dtd->wave & (0x1 << heartbeat->idx)) == 0)
		v ^= mask;

	gpio_write(dti, v, mask);

	heartbeat->time_ms = time_ms;
	heartbeat->idx = (heartbeat->idx + 1) & 0xf;
}
