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
static size_t ack(size_t remaining, void *hw);

static void idle(bool addressable, bool stop, void *hw);
static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw);

static size_t read_bytes(uint8_t *buf, size_t n, void *hw);
static size_t write_bytes(uint8_t *buf, size_t n, bool last, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	i2c_ops_t ops;


	ops.configure = configure;
	ops.state = state;
	ops.start = start;
	ops.ack = ack;
	ops.idle = idle;
	ops.connect = connect;
	ops.read_bytes = read_bytes;
	ops.write_bytes = write_bytes;
	ops.read = 0x0;
	ops.write = 0x0;

	return i2c_create(&ops, &dtd->cfg, dtd);
}

driver_probe("avr,i2c", probe);

static int configure(i2c_cfg_t *cfg, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;
	i2c_regs_t *regs = dtd->regs;
	uint8_t brate;


	/* check config */
	// max 400 kHz
	if(cfg->clock_khz > 400)
		return_errno(E_LIMIT);

	// only 7-bit addresses
	if(cfg->addr & 0x80)
		return_errno(E_LIMIT);

	/* compute baud rate */
	// NOTE assumption: TWSR[TWPS] = 0
	brate = (((AVR_IO_CLOCK_HZ / 1000) / cfg->clock_khz) - 16) / 2;

	if(brate == 0)
		return_errno(E_INVAL);

	/* disable twi */
	*dtd->prr |= dtd->prr_en;

	/* re-enable twi */
	*dtd->prr &= ~dtd->prr_en;

	regs->twsr = 0x0;
	regs->twbr = brate;
	regs->twamr = 0x0;
	regs->twar = cfg->addr << 0x1 | (((bool)cfg->bcast_en) << TWAR_TWGCE);
	regs->twcr = (0x1 << TWCR_TWEN)
			   | (0x0 << TWCR_TWEA)
			   | (((bool)cfg->int_num) << TWCR_TWIE)
			   ;

	return 0;
}

static i2c_state_t state(void *hw){
	i2c_regs_t *regs = ((dt_data_t*)hw)->regs;


	if((regs->twcr & (0x1 << TWCR_TWINT)) == 0)
		return I2C_STATE_NONE;

	switch(regs->twsr & ~(0x3 << TWSR_TWPS)){
	case 0x08:	return I2C_STATE_MST_START;
	case 0x10:	return I2C_STATE_MST_START;
	case 0x20:	return I2C_STATE_MST_START_NACK;
	case 0x48:	return I2C_STATE_MST_START_NACK;
	case 0x18:	return I2C_STATE_MST_WR_ACK;
	case 0x28:	return I2C_STATE_MST_WR_DATA_ACK;
	case 0x30:	return I2C_STATE_MST_WR_DATA_NACK;
	case 0x40:	return I2C_STATE_MST_RD_ACK;
	case 0x50:	return I2C_STATE_MST_RD_DATA_ACK;
	case 0x58:	return I2C_STATE_MST_RD_DATA_NACK;
	case 0x38:	return I2C_STATE_MST_ARBLOST;
	case 0x60:	return I2C_STATE_SLA_RD_MATCH;
	case 0x68:	return I2C_STATE_SLA_RD_MATCH;
	case 0x70:	return I2C_STATE_SLA_RD_MATCH;
	case 0x78:	return I2C_STATE_SLA_RD_MATCH;
	case 0x80:	return I2C_STATE_SLA_RD_DATA_ACK;
	case 0x90:	return I2C_STATE_SLA_RD_DATA_ACK;
	case 0x88:	return I2C_STATE_SLA_RD_DATA_NACK;
	case 0x98:	return I2C_STATE_SLA_RD_DATA_NACK;
	case 0xa0:	return I2C_STATE_SLA_RD_STOP;
	case 0xa8:	return I2C_STATE_SLA_WR_MATCH;
	case 0xb0:	return I2C_STATE_SLA_WR_MATCH;
	case 0xb8:	return I2C_STATE_SLA_WR_DATA_ACK;
	case 0xc8:	return I2C_STATE_SLA_WR_DATA_ACK;
	case 0xc0:	return I2C_STATE_SLA_WR_DATA_NACK;
	case 0x00:	return I2C_STATE_ERROR;
	default:	return I2C_STATE_ERROR;
	}
}

static void start(void *hw){
	i2c_regs_t *regs = ((dt_data_t*)hw)->regs;


	regs->twcr |= (0x1 << TWCR_TWSTA)
			   |  (0x1 << TWCR_TWINT)
			   ;
}

static size_t ack(size_t remaining, void *hw){
	i2c_regs_t *regs = ((dt_data_t*)hw)->regs;


	regs->twcr &= ~(0x1 << TWCR_TWEA);
	regs->twcr |= ((remaining > 1) << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	return 1;
}

static void idle(bool addressable, bool stop, void *hw){
	i2c_regs_t *regs = ((dt_data_t*)hw)->regs;


	regs->twcr = (regs->twcr & (0x1 << TWCR_TWIE))
			   | (addressable << TWCR_TWEA)
			   | (0x1 << TWCR_TWEN)
			   | (0x1 << TWCR_TWINT)
			   | (stop << TWCR_TWSTO)
			   ;
}

static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw){
	i2c_regs_t *regs = ((dt_data_t*)hw)->regs;


	regs->twdr = (slave << TWDR_ADDR) | (((bool)(cmd & I2C_CMD_READ)) << TWDR_RW);
	regs->twcr = (regs->twcr & (0x1 << TWCR_TWIE))
			   | (0x1 << TWCR_TWEN)
			   | (0x1 << TWCR_TWINT)
			   ;
}

static size_t read_bytes(uint8_t *buf, size_t n, void *hw){
	buf[0] = ((dt_data_t*)hw)->regs->twdr;

	return 1;
}

static size_t write_bytes(uint8_t *buf, size_t n, bool last, void *hw){
	((dt_data_t*)hw)->regs->twdr = buf[0];

	// only the last byte must not be acknowledged
	return ack((last && n == 1) ? 1 : 2, hw);
}
