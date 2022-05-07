/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H


#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/i2c.h>
#include <sys/linebuf.h>


/* types */
typedef enum{
	// state sections bits
	I2C_STATE_BIT_SPECIAL = 0x20,
	I2C_STATE_BIT_MASTER = 0x40,
	I2C_STATE_BIT_SLAVE = 0x80,

	// special states
	I2C_STATE_NEXT_CMD = I2C_STATE_BIT_SPECIAL | 0x01,
	I2C_STATE_ERROR,
	I2C_STATE_INVAL,
	I2C_STATE_NONE,

	// master mode states
	I2C_STATE_MST_START = I2C_STATE_BIT_MASTER | 0x01,
	I2C_STATE_MST_RESTART,

	I2C_STATE_MST_SLAW_ACK,
	I2C_STATE_MST_SLAW_NACK,
	I2C_STATE_MST_SLAW_DATA_ACK,
	I2C_STATE_MST_SLAW_DATA_NACK,

	I2C_STATE_MST_SLAR_ACK,
	I2C_STATE_MST_SLAR_NACK,
	I2C_STATE_MST_SLAR_DATA_ACK,
	I2C_STATE_MST_SLAR_DATA_NACK,

	I2C_STATE_MST_ARBLOST,

	// slave mode states
	I2C_STATE_SLA_SLAW_MATCH = I2C_STATE_BIT_SLAVE | 0x01,
	I2C_STATE_SLA_SLAW_ARBLOST_ADDR_MATCH,
	I2C_STATE_SLA_SLAR_ADDR_MATCH,
	I2C_STATE_SLA_SLAR_ARBLOST_ADDR_MATCH,
	I2C_STATE_SLA_BCAST_MATCH,
	I2C_STATE_SLA_BCAST_ARBLOST_MATCH,

	I2C_STATE_SLA_SLAW_DATA_ACK,
	I2C_STATE_SLA_SLAW_DATA_NACK,
	I2C_STATE_SLA_BCAST_DATA_ACK,
	I2C_STATE_SLA_BCAST_DATA_NACK,

	I2C_STATE_SLA_SLAW_STOP,

	I2C_STATE_SLA_SLAR_DATA_ACK,
	I2C_STATE_SLA_SLAR_DATA_NACK,
	I2C_STATE_SLA_SLAR_DATA_LAST_ACK,
} i2c_state_t;

typedef enum{
	I2C_CMD_READ = 1,
	I2C_CMD_WRITE,
} i2c_cmd_t;

typedef struct{
	i2c_cmd_t cmd;
	uint8_t slave;

	linebuf_t data;
} i2c_dgram_t;

typedef struct{
	int (*configure)(i2c_cfg_t *cfg, bool int_en, void *hw);

	i2c_state_t (*state)(void *hw);

	void (*start)(void *hw);
	void (*ack)(bool ack, void *hw);

	void (*slave_mode)(bool addressable, bool stop, void *hw);
	void (*slave_addr)(i2c_cmd_t cmd, uint8_t slave, void *hw);

	uint8_t (*byte_read)(void *hw);
	void (*byte_write)(uint8_t c, bool last, void *hw);

	void *hw;
	int_num_t int_num;
} i2c_ops_t;

typedef struct{
	i2c_cfg_t *cfg;
	i2c_ops_t *ops;

	itask_queue_t master_cmds,
				  slave_cmds;

	mutex_t mtx;
} i2c_t;


/* prototypes */
i2c_t *i2c_create(i2c_ops_t *ops, i2c_cfg_t *cfg, void *hw);
void i2c_destroy(i2c_t *i2c);

size_t i2c_read(i2c_t *i2c, uint8_t slave, void *buf, size_t n);
size_t i2c_write(i2c_t *i2c, uint8_t slave, void *buf, size_t n);


#endif // DRIVER_I2C_H
