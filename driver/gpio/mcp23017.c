/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <sys/compiler.h>
#include <sys/string.h>
#include <sys/gpio.h>


/* macros */
#define	NUM_PORTS	2

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
#define MCP_READ(mcp, buf, n) \
	i2c_read((mcp)->hw, (mcp)->slave, buf, n)

#define MCP_WRITE(mcp, buf) \
	i2c_write((mcp)->hw, (mcp)->slave, buf, sizeof_array(buf))

#define MCP_DATA(...)	((uint8_t []){ __VA_ARGS__ })


/* types */
typedef enum{
	PORT_A = 0,
	PORT_B,
} port_num_t;

typedef struct{
	uint8_t num;		/**< set by the driver, cf. port_num_t */
	gpio_cfg_t cfg;

	uint8_t slave;
	i2c_t *hw;			/**< set by the driver */
} __packed dev_data_t;

typedef struct{
	dev_data_t ports[NUM_PORTS];
} dt_data_t;


/* local/static prototypes */
static gpio_int_t read(void *hw);
static int write(gpio_int_t v, void *hw);

static int configure(dev_data_t *mcp, gpio_cfg_t *cfg);

static gpio_int_t rx(dev_data_t *mcp, uint8_t addr);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	gpio_t *ports[NUM_PORTS] = { 0x0 };
	size_t name_len = strlen(name);
	char port_name[name_len + 2];
	gpio_ops_t ops;


	/* amend port configuration */
	dtd->ports[0].num = PORT_A;
	dtd->ports[0].hw = dt_itf;
	dtd->ports[1].num = PORT_B;
	dtd->ports[1].hw = dt_itf;

	/* create and configure gpio devices */
	ops.read = read;
	ops.write = write;

	strcpy(port_name, name);
	port_name[name_len + 1] = 0;

	for(uint8_t i=0; i<NUM_PORTS; i++){
		port_name[name_len] = 'a' + i;
		ports[i] = gpio_create(port_name, &ops, &dtd->ports[i].cfg, dtd->ports + i);

		if(ports[i] == 0x0 || configure(dtd->ports + i, &dtd->ports[i].cfg) != 0)
			goto err;
	}

	return 0x0;


err:
	for(uint8_t i=0; i<NUM_PORTS; i++){
		if(ports[i] != 0x0)
			gpio_destroy(ports[i]);
	}

	return 0x0;
}

driver_probe("mcp23017", probe);

static gpio_int_t read(void *hw){
	return rx(hw, GPIO);
}

static int write(gpio_int_t v, void *hw){
	dev_data_t *mcp;


	mcp = (dev_data_t*)hw;

	return MCP_WRITE(mcp, MCP_DATA(GPIO + mcp->num, v));
}

static int configure(dev_data_t *mcp, gpio_cfg_t *cfg){
	int r = 0;


	// device config
	r |= MCP_WRITE(mcp,
		MCP_DATA(IOCON + mcp->num,
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
	r |= MCP_WRITE(mcp, MCP_DATA(IODIR + mcp->num, cfg->in_mask));
	r |= MCP_WRITE(mcp, MCP_DATA(IPOL + mcp->num, 0x0));
	r |= MCP_WRITE(mcp, MCP_DATA(GPPU + mcp->num, cfg->in_mask));

	// interrupt config
	r |= MCP_WRITE(mcp, MCP_DATA(INTCON + mcp->num, 0x0));
	r |= MCP_WRITE(mcp, MCP_DATA(DEFVAL + mcp->num, 0x0));
	r |= MCP_WRITE(mcp, MCP_DATA(GPINTEN + mcp->num, cfg->int_mask));

	// clear interrupts
	rx(mcp, INTCAP);

	return -r;
}

static gpio_int_t rx(dev_data_t *mcp, uint8_t addr){
	gpio_int_t v;


	MCP_WRITE(mcp, MCP_DATA(addr + mcp->num));
	MCP_READ(mcp, &v, 1);

	return v;
}
