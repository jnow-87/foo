/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <driver/gpio.h>
#include <sys/errno.h>
#include <sys/vector.h>


/* types */
typedef struct{
	gpio_cfg_t port;
	uint8_t int_num;				/**< cf. int_num_t */

	gpio_itf_t *gpio;				/**< set by the driver */
} dt_data_t;


/* local/static prototypes */
static void int_hdlr(int_num_t num, void *payload);


/* local functions */
void *probe(char const *name, void *dt_data, void *dt_itf){
	gpio_itf_t *dti = (gpio_itf_t*)dt_itf;
	dt_data_t *dtd = (dt_data_t*)dt_data;


	if(dtd->int_num == 0)
		goto_errno(end, E_INVAL);

	if(dti->ops.configure(&dtd->port, dti->dt_data, dti->payload) != 0)
		goto end;

	if(vector_add(&dti->bcast_ints, &dtd->port.int_num) != 0)
		goto end;

	dtd->gpio = dti;
	int_register(dtd->port.int_num, int_hdlr, dtd);

end:
	return 0x0;
}

driver_probe("gpio,intfwd", probe);

static void int_hdlr(int_num_t num, void *payload){
	dt_data_t *dtd = (dt_data_t*)payload;
	gpio_itf_t *itf = dtd->gpio;


	if((itf->ops.read(itf->dt_data, itf->payload) ^ dtd->port.invert_mask) & dtd->port.int_mask)
		int_foretell(dtd->int_num);
}
