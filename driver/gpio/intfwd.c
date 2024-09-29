/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <driver/gpio.h>
#include <sys/errno.h>
#include <sys/types.h>


/* types */
typedef struct{
	uint8_t pin;
	uint8_t int_num;	/**< cf. int_num_t */
} dt_data_t;


/* local functions */
void *probe(char const *name, void *dt_data, void *dt_itf){
	gpio_t *dti = (gpio_t*)dt_itf;
	dt_data_t *dtd = (dt_data_t*)dt_data;


	if(dtd->int_num == 0)
		goto_errno(end, E_INVAL);

	(void)gpio_int_register(dti, dtd->int_num, (0x1 << dtd->pin));

end:
	return 0x0;
}

driver_probe("gpio,intfwd", probe);
