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
#include <sys/errno.h>
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
} dt_data_t;

typedef struct{
	dt_data_t *dtd;
	i2c_t *dti;

	gpio_itf_t itf;
} dev_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *hw);
static intgpio_t read(void *hw);
static int write(intgpio_t v, void *hw);

static int reg_read(uint8_t reg, uint8_t *v, dev_data_t *gpio);
static int reg_write(uint8_t reg, uint8_t v, dev_data_t *gpio);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *gpio;


	gpio = kmalloc(sizeof(dev_data_t));

	if(gpio == 0x0)
		return 0x0;

	gpio->dtd = dt_data;
	gpio->dti = dt_itf;

	gpio->itf.configure = configure;
	gpio->itf.read = read;
	gpio->itf.write = write;

	gpio->itf.hw = gpio;

	return &gpio->itf;
}

driver_probe("mcp23017", probe);

static int configure(gpio_cfg_t *cfg, void *hw){
	int r = 0;
	dev_data_t *gpio = (dev_data_t*)hw;
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

	r |= reg_write(IOCON, v, gpio);

	// port config
	r |= reg_write(IODIR, cfg->in_mask, gpio);
	r |= reg_write(IPOL, 0x0, gpio);
	r |= reg_write(GPPU, cfg->in_mask, gpio);

	r |= reg_write(GPIO, cfg->out_mask & cfg->invert_mask, gpio);

	// interrupt config
	r |= reg_write(INTCON, 0x0, gpio);
	r |= reg_write(DEFVAL, 0x0, gpio);
	r |= reg_write(GPINTEN, cfg->int_mask, gpio);

	// clear interrupts
	r |= reg_read(INTCAP, &v, gpio);

	return -r;
}

static intgpio_t read(void *hw){
	uint8_t v;


	(void)reg_read(GPIO, &v, hw);

	return v;
}

static int write(intgpio_t v, void *hw){
	return reg_write(GPIO, v, hw);
}

static int reg_read(uint8_t reg, uint8_t *v, dev_data_t *gpio){
	int r = 0;
	dt_data_t *dtd = gpio->dtd;


	r |= (i2c_write(gpio->dti, I2C_MASTER, dtd->slave_addr, ((uint8_t []){ reg + dtd->port }), 1) != 1);
	r |= (i2c_read(gpio->dti, I2C_MASTER, dtd->slave_addr, v, 1) != 1);

	if(r != 0)
		return_errno(errno ? errno : E_IO);

	return 0;
}

static int reg_write(uint8_t reg, uint8_t v, dev_data_t *gpio){
	if(i2c_write(gpio->dti, I2C_MASTER, gpio->dtd->slave_addr, ((uint8_t []){ reg + gpio->dtd->port, v }), 2) != 2)
		return_errno(errno ? errno : E_IO);

	return 0;
}
