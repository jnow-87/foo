/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <sys/compiler.h>
#include <sys/gpio.h>


/* macros */
// registers
#define IODIR	0x00
#define IPOL	0x02
#define GPINTEN	0x04
#define DEFVAL	0x06
#define INTCON	0x08
#define IOCON	0x0a
#define GPPU	0x0c
#define INTF	0x0e
#define INTCAP	0x10
#define GPIO	0x12
#define OLAT	0x14

// bits
#define IOCON_BANK		7
#define IOCON_MIRROR	6
#define IOCON_SEQOP		5
#define IOCON_DISSLW	4
#define IOCON_HAEN		3
#define IOCON_ODR		2
#define IOCON_INTPOL	1


/* types */
typedef enum{
	PORT_A = 0,
	PORT_B,
} port_t;

typedef struct{
	uint8_t port;		/**< cf. port_t */
	uint8_t slave_addr;
	uint8_t int_num;	/**< cf. int_num_t */
} dt_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload);
static intgpio_t read(void *dt_data, void *payload);
static int write(intgpio_t v, void *dt_data, void *payload);

static int reg_read(uint8_t reg, uint8_t *v, dt_data_t *dtd, i2c_t *i2c);
static int reg_write(uint8_t reg, uint8_t v, dt_data_t *dtd, i2c_t *i2c);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_ops_t ops;


	ops.configure = configure;
	ops.read = read;
	ops.write = write;

	return gpio_itf_create(&ops, dtd->int_num, dtd, &dt_itf, sizeof(i2c_t*));
}

driver_probe("mcp23017", probe);

static int configure(gpio_cfg_t *cfg, void *dt_data, void *payload){
	int r = 0;
	dt_data_t *dtd = (dt_data_t*)dt_data;
	i2c_t *i2c = *((i2c_t**)payload);
	uint8_t v;


	// device config
	v = (0x0 << IOCON_BANK)
	  | (0x0 << IOCON_MIRROR)
	  | (0x1 << IOCON_SEQOP)
	  | (0x0 << IOCON_DISSLW)
	  | (0x0 << IOCON_HAEN)
	  | (0x0 << IOCON_ODR)
	  | (0x1 << IOCON_INTPOL)
	  ;

	r |= reg_write(IOCON, v, dtd, i2c);

	// port config
	r |= reg_write(IODIR, cfg->in_mask, dtd, i2c);
	r |= reg_write(IPOL, 0x0, dtd, i2c);
	r |= reg_write(GPPU, cfg->in_mask, dtd, i2c);

	r |= reg_write(GPIO, cfg->out_mask & cfg->invert_mask, dtd, i2c);

	// interrupt config
	r |= reg_write(INTCON, 0x0, dtd, i2c);
	r |= reg_write(DEFVAL, 0x0, dtd, i2c);
	r |= reg_write(GPINTEN, cfg->int_mask, dtd, i2c);

	// clear interrupts
	r |= reg_read(INTCAP, &v, dtd, i2c);

	return -r;
}

static intgpio_t read(void *dt_data, void *payload){
	uint8_t v;


	(void)reg_read(GPIO, &v, dt_data, *((i2c_t**)payload));

	return v;
}

static int write(intgpio_t v, void *dt_data, void *payload){
	return reg_write(GPIO, v, (dt_data_t*)dt_data, *((i2c_t**)payload));
}

static int reg_read(uint8_t reg, uint8_t *v, dt_data_t *dtd, i2c_t *i2c){
	int r = 0;


	r |= (i2c_write(i2c, I2C_MASTER, dtd->slave_addr, ((uint8_t []){ reg + dtd->port }), 1) != 1);
	r |= (i2c_read(i2c, I2C_MASTER, dtd->slave_addr, v, 1) != 1);

	if(r != 0)
		return_errno(errno ? errno : E_IO);

	return 0;
}

static int reg_write(uint8_t reg, uint8_t v, dt_data_t *dtd, i2c_t *i2c){
	if(i2c_write(i2c, I2C_MASTER, dtd->slave_addr, ((uint8_t []){ reg + dtd->port, v }), 2) != 2)
		return_errno(errno ? errno : E_IO);

	return 0;
}
