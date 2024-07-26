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
#define MCP_READ(i2c, dtd, buf, n)	(i2c_read(i2c, I2C_MASTER, (dtd)->slave_addr, buf, n) != n)
#define MCP_WRITE(i2c, dtd, buf)	(i2c_write(i2c, I2C_MASTER, (dtd)->slave_addr, buf, sizeof_array(buf)) != sizeof_array(buf))
#define MCP_DATA(...)				((uint8_t []){ __VA_ARGS__ })


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

static intgpio_t reg_read(uint8_t reg, dt_data_t *dtd, i2c_t *i2c);


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


	// device config
	r |= MCP_WRITE(i2c, dtd,
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
	r |= MCP_WRITE(i2c, dtd, MCP_DATA(IODIR + dtd->port, cfg->in_mask));
	r |= MCP_WRITE(i2c, dtd, MCP_DATA(IPOL + dtd->port, 0x0));
	r |= MCP_WRITE(i2c, dtd, MCP_DATA(GPPU + dtd->port, cfg->in_mask));

	r |= MCP_WRITE(i2c, dtd, MCP_DATA(GPIO + dtd->port, cfg->out_mask & cfg->invert_mask));

	// interrupt config
	r |= MCP_WRITE(i2c, dtd, MCP_DATA(INTCON + dtd->port, 0x0));
	r |= MCP_WRITE(i2c, dtd, MCP_DATA(DEFVAL + dtd->port, 0x0));
	r |= MCP_WRITE(i2c, dtd, MCP_DATA(GPINTEN + dtd->port, cfg->int_mask));

	// clear interrupts
	reg_read(INTCAP, dtd, i2c);

	return -r;
}

static intgpio_t read(void *dt_data, void *payload){
	return reg_read(GPIO, dt_data, *((i2c_t**)payload));
}

static int write(intgpio_t v, void *dt_data, void *payload){
	dt_data_t *dtd = (dt_data_t*)dt_data;


	return MCP_WRITE(*((i2c_t**)payload), dtd, MCP_DATA(GPIO + dtd->port, v));
}

static intgpio_t reg_read(uint8_t reg, dt_data_t *dtd, i2c_t *i2c){
	intgpio_t v;


	MCP_WRITE(i2c, dtd, MCP_DATA(reg + dtd->port));
	MCP_READ(i2c, dtd, &v, 1);

	return v;
}
