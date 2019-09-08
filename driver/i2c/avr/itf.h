/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_I2C_ITF_H
#define AVR_I2C_ITF_H


#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/inttask.h>
#include <driver/i2c.h>
#include <sys/ringbuf.h>


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

// instructions
#define STATUS(regs) \
	(regs)->twsr & ~(0x3 << TWSR_TWPS)

#define WAITINT(regs)({ \
	while(!((regs)->twcr & (0x1 << TWCR_TWINT))); \
	STATUS(regs); \
})


/* types */
typedef enum{
	// master mode status
	S_MST_START = 0x08,
	S_MST_RESTART = 0x10,

	S_MST_SLAW_ACK = 0x18,
	S_MST_SLAW_NACK = 0x20,
	S_MST_SLAW_DATA_ACK = 0x28,
	S_MST_SLAW_DATA_NACK = 0x30,

	S_MST_SLAR_ACK = 0x40,
	S_MST_SLAR_NACK = 0x48,
	S_MST_SLAR_DATA_ACK = 0x50,
	S_MST_SLAR_DATA_NACK = 0x58,

	S_MST_ARBLOST = 0x38,

	// slave mode status
	S_SLA_SLAW_MATCH = 0x60,
	S_SLA_SLAW_ARBLOST_ADDR_MATCH = 0x68,
	S_SLA_SLAR_ADDR_MATCH = 0xa8,
	S_SLA_SLAR_ARBLOST_ADDR_MATCH = 0xb0,
	S_SLA_BCAST_MATCH = 0x70,
	S_SLA_BCAST_ARBLOST_MATCH = 0x78,

	S_SLA_SLAW_DATA_ACK = 0x80,
	S_SLA_SLAW_DATA_NACK = 0x88,
	S_SLA_BCAST_DATA_ACK = 0x90,
	S_SLA_BCAST_DATA_NACK = 0x98,

	S_SLA_SLAW_STOP = 0xa0,

	S_SLA_SLAR_DATA_ACK = 0xb8,
	S_SLA_SLAR_DATA_NACK = 0xc0,
	S_SLA_SLAR_DATA_LAST_ACK = 0xc8,

	// misc
	S_ERROR = 0x0,
	S_INVAL = 0xf8,
	S_NONE = 0x1,		// artificial state introduced by the driver
	S_NEXT_CMD = 0x2,	// artificial state introduced by the driver
						// to process the next interrupt command
} status_t;

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

	uint8_t const int_num;
} dt_data_t;

typedef struct{
	i2c_itf_t hw;
	dt_data_t *dtd;

	mutex_t mtx;

	itask_queue_t master_cmd_queue,
				  slave_cmd_queue;
	ringbuf_t slave_rx_buf;
} avr_i2c_t;


/* prototypes */
#ifdef CONFIG_I2C_AVR_POLLING
int avr_i2c_master_read_poll(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
int avr_i2c_master_write_poll(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
int avr_i2c_slave_read_poll(uint8_t *buf, size_t n, void *data);
int avr_i2c_slave_write_poll(uint8_t *buf, size_t n, void *data);
#endif // CONFIG_I2C_AVR_POLLING

#ifdef CONFIG_I2C_AVR_INTERRUPT
int avr_i2c_master_read_int(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
int avr_i2c_master_write_int(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
int avr_i2c_slave_read_int(uint8_t *buf, size_t n, void *data);
int avr_i2c_slave_write_int(uint8_t *buf, size_t n, void *data);

void avr_i2c_int_hdlr(int_num_t num, void *data);
#endif // CONFIG_I2C_AVR_INTERRUPT


#endif // AVR_I2C_ITF_H
