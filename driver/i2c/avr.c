/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/driver.h>
#include <driver/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>


/* macros */
// register bits
#define TWCR_TWINT		7
#define TWCR_TWEA		6
#define TWCR_TWSTA		5
#define TWCR_TWSTO		4
#define TWCR_TWWC		3
#define TWCR_TWEN		2
#define TWCR_TWIE		0

#define TWSR_TWS		3
#define TWSR_TWPS		0

#define TWBR_BR			0

#define TWDR_ADDR		1
#define TWDR_RW			0

#define TWAR_TWA		1
#define TWAR_TWGCE		0

#define TWAMR_TWAM		1


/* types */
typedef struct{
	uint8_t volatile twbr,
					 twsr,
					 twar,
					 twdr,
					 twcr,
					 twamr;
} i2c_regs_t;

typedef struct{
	// device registers
	i2c_regs_t *regs;

	// power control
	uint8_t volatile *prr;
	uint8_t const prr_en;		// PRR device enable value (bit mask)

	i2c_cfg_t cfg;
} dt_data_t;


/* local/static prototypes */
static int configure(i2c_cfg_t *cfg, void *hw);

static i2c_state_t state(void *hw);
static void start(void *hw);
static void ack(bool ack, void *hw);

static void slave_mode(bool addressable, bool stop, void *hw);
static void slave_addr(i2c_cmd_t cmd, uint8_t slave, void *hw);

static uint8_t byte_read(void *hw);
static void byte_write(uint8_t c, bool ack, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd;
	i2c_ops_t ops;


	dtd = (dt_data_t*)dt_data;

	ops.configure = configure;
	ops.state = state;
	ops.start = start;
	ops.ack = ack;
	ops.slave_mode = slave_mode;
	ops.slave_addr = slave_addr;
	ops.byte_read = byte_read;
	ops.byte_write = byte_write;
	ops.read = 0x0;
	ops.write = 0x0;

	return i2c_create(&ops, &dtd->cfg, dtd);
}

driver_probe("avr,i2c", probe);

static int configure(i2c_cfg_t *cfg, void *hw){
	uint8_t brate;
	dt_data_t *dtd;
	i2c_regs_t *regs;


	dtd = (dt_data_t*)hw;
	regs = dtd->regs;

	/* check config */
	// max 400 kHz
	if(cfg->clock_khz > 400)
		return_errno(E_LIMIT);

	// only 7-bit addresses
	if(cfg->addr & 0x80)
		return_errno(E_LIMIT);

	/* compute baud rate */
	// NOTE assumption: TWSR[TWPS] = 0
	brate = ((AVR_IO_CLOCK_HZ / (cfg->clock_khz * 1000)) - 16) / 2;

	if(brate == 0)
		return_errno(E_INVAL);

	/* disable twi */
	*dtd->prr |= dtd->prr_en;

	/* re-enable twi */
	*dtd->prr &= ~dtd->prr_en;

	regs->twsr = 0x0;
	regs->twbr = brate;
	regs->twamr = 0x0;
	regs->twar = cfg->addr << 0x1 | ((cfg->bcast_en ? 0x1 : 0x0) << TWAR_TWGCE);
	regs->twcr = (0x1 << TWCR_TWEN)
			   | (0x0 << TWCR_TWEA)
			   | ((cfg->int_num ? 0x1 : 0x0) << TWCR_TWIE)
			   ;

	return E_OK;
}

static i2c_state_t state(void *hw){
	i2c_regs_t *regs;


	regs = ((dt_data_t*)hw)->regs;

	if((regs->twcr & (0x1 << TWCR_TWINT)) == 0)
		return I2C_STATE_NONE;

	switch(regs->twsr & ~(0x3 << TWSR_TWPS)){
	case 0x08:	return I2C_STATE_MST_START;
	case 0x10:	return I2C_STATE_MST_RESTART;
	case 0x18:	return I2C_STATE_MST_SLAW_ACK;
	case 0x20:	return I2C_STATE_MST_SLAW_NACK;
	case 0x28:	return I2C_STATE_MST_SLAW_DATA_ACK;
	case 0x30:	return I2C_STATE_MST_SLAW_DATA_NACK;
	case 0x40:	return I2C_STATE_MST_SLAR_ACK;
	case 0x48:	return I2C_STATE_MST_SLAR_NACK;
	case 0x50:	return I2C_STATE_MST_SLAR_DATA_ACK;
	case 0x58:	return I2C_STATE_MST_SLAR_DATA_NACK;
	case 0x38:	return I2C_STATE_MST_ARBLOST;
	case 0x60:	return I2C_STATE_SLA_SLAW_MATCH;
	case 0x68:	return I2C_STATE_SLA_SLAW_ARBLOST_ADDR_MATCH;
	case 0xa8:	return I2C_STATE_SLA_SLAR_ADDR_MATCH;
	case 0xb0:	return I2C_STATE_SLA_SLAR_ARBLOST_ADDR_MATCH;
	case 0x70:	return I2C_STATE_SLA_BCAST_MATCH;
	case 0x78:	return I2C_STATE_SLA_BCAST_ARBLOST_MATCH;
	case 0x80:	return I2C_STATE_SLA_SLAW_DATA_ACK;
	case 0x88:	return I2C_STATE_SLA_SLAW_DATA_NACK;
	case 0x90:	return I2C_STATE_SLA_BCAST_DATA_ACK;
	case 0x98:	return I2C_STATE_SLA_BCAST_DATA_NACK;
	case 0xa0:	return I2C_STATE_SLA_SLAW_STOP;
	case 0xb8:	return I2C_STATE_SLA_SLAR_DATA_ACK;
	case 0xc0:	return I2C_STATE_SLA_SLAR_DATA_NACK;
	case 0xc8:	return I2C_STATE_SLA_SLAR_DATA_LAST_ACK;
	case 0x00:	return I2C_STATE_ERROR;
	default:	return I2C_STATE_INVAL;
	}
}

static void start(void *hw){
	i2c_regs_t *regs;


	regs = ((dt_data_t*)hw)->regs;

	regs->twcr |= (0x1 << TWCR_TWSTA)
			   |  (0x1 << TWCR_TWINT)
			   ;
}

static void ack(bool ack, void *hw){
	i2c_regs_t *regs;


	regs = ((dt_data_t*)hw)->regs;

	regs->twcr &= ~(0x1 << TWCR_TWEA);
	regs->twcr |= ((ack ? 0x1 : 0x0) << TWCR_TWEA) | (0x1 << TWCR_TWINT);
}

static void slave_mode(bool addressable, bool stop, void *hw){
	i2c_regs_t *regs;


	regs = ((dt_data_t*)hw)->regs;

	regs->twcr = (regs->twcr & (0x1 << TWCR_TWIE))
			   | ((addressable ? 0x1 : 0x0) << TWCR_TWEA)
			   | (0x1 << TWCR_TWEN)
			   | (0x1 << TWCR_TWINT)
			   | ((stop ? 0x1 : 0x0)  << TWCR_TWSTO)
			   ;
}

static void slave_addr(i2c_cmd_t cmd, uint8_t slave, void *hw){
	i2c_regs_t *regs;


	regs = ((dt_data_t*)hw)->regs;

	regs->twdr = (slave << TWDR_ADDR) | (((cmd & I2C_CMD_READ) ? 0x1 : 0x0) << TWDR_RW);
	regs->twcr = (regs->twcr & (0x1 << TWCR_TWIE))
			   | (0x1 << TWCR_TWEN)
			   | (0x1 << TWCR_TWINT)
			   ;
}

static uint8_t byte_read(void *hw){
	return ((dt_data_t*)hw)->regs->twdr;
}

static void byte_write(uint8_t c, bool last, void *hw){
	((dt_data_t*)hw)->regs->twdr = c;
	ack(!last, hw);
}
