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

// I/O wrapper
#define MCP_READ(gpio, buf, n)	i2c_read((gpio)->dti, (gpio)->dtd->slave_addr, buf, n)
#define MCP_WRITE(gpio, buf)	i2c_write((gpio)->dti, (gpio)->dtd->slave_addr, buf, sizeof_array(buf))
#define MCP_DATA(...)			((uint8_t []){ __VA_ARGS__ })


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
	i2c_t *dti;
	dt_data_t *dtd;
	gpio_itf_t itf;
} dev_data_t;


/* local/static prototypes */
static int configure(gpio_cfg_t *cfg, void *hw);
static intgpio_t read(void *hw);
static int write(intgpio_t v, void *hw);

static intgpio_t reg_read(uint8_t reg, dev_data_t *gpio);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *gpio;
	gpio_itf_t *itf;


	gpio = kmalloc(sizeof(dev_data_t));

	if(gpio == 0x0)
		return 0x0;

	gpio->dti = dt_itf;
	gpio->dtd = dt_data;

	itf = &gpio->itf;
	itf->configure = configure;
	itf->read = read;
	itf->write = write;
	itf->hw = gpio;

	return itf;
}

driver_probe("mcp23017", probe);

static int configure(gpio_cfg_t *cfg, void *hw){
	int r = 0;
	dev_data_t *gpio = (dev_data_t*)hw;
	dt_data_t *dtd = gpio->dtd;


	// device config
	r |= MCP_WRITE(gpio,
		MCP_DATA(IOCON + dtd->port,
			  (0x0 << IOCON_BANK)
			| (0x0 << IOCON_MIRROR)
			| (0x1 << IOCON_SEQOP)
			| (0x0 << IOCON_DISSLW)
			| (0x0 << IOCON_HAEN)
			| (0x0 << IOCON_ODR)
			| (0x1 << IOCON_INTPOL)
		)
	);

	// port config
	r |= MCP_WRITE(gpio, MCP_DATA(IODIR + dtd->port, cfg->in_mask));
	r |= MCP_WRITE(gpio, MCP_DATA(IPOL + dtd->port, 0x0));
	r |= MCP_WRITE(gpio, MCP_DATA(GPPU + dtd->port, cfg->in_mask));

	r |= MCP_WRITE(gpio, MCP_DATA(GPIO + dtd->port, cfg->out_mask & cfg->invert_mask));

	// interrupt config
	r |= MCP_WRITE(gpio, MCP_DATA(INTCON + dtd->port, 0x0));
	r |= MCP_WRITE(gpio, MCP_DATA(DEFVAL + dtd->port, 0x0));
	r |= MCP_WRITE(gpio, MCP_DATA(GPINTEN + dtd->port, cfg->int_mask));

	// clear interrupts
	reg_read(INTCAP, gpio);

	return -r;
}

static intgpio_t read(void *hw){
	return reg_read(GPIO, hw);
}

static int write(intgpio_t v, void *hw){
	dev_data_t *gpio = (dev_data_t*)hw;


	return MCP_WRITE(gpio, MCP_DATA(GPIO + gpio->dtd->port, v));
}

static intgpio_t reg_read(uint8_t reg, dev_data_t *gpio){
	intgpio_t v;


	MCP_WRITE(gpio, MCP_DATA(reg + gpio->dtd->port));
	MCP_READ(gpio, &v, 1);

	return v;
}
