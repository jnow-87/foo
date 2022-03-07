/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H


#include <config/config.h>
#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/i2c.h>
#include <sys/ringbuf.h>


/* types */
typedef enum{
	// state sections bits
	I2C_STATE_BIT_SPECIAL = 0x20,
	I2C_STATE_BIT_MASTER = 0x40,
	I2C_STATE_BIT_SLAVE = 0x80,

	// special states
	I2C_STATE_ERROR = I2C_STATE_BIT_SPECIAL | 0x01,
	I2C_STATE_INVAL,
	I2C_STATE_NONE,

	// master mode states
	I2C_STATE_MST_NEXT_CMD = I2C_STATE_BIT_MASTER | 0x01,
	I2C_STATE_MST_NOINT,
	I2C_STATE_MST_START,
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
	I2C_CMD_SLAVE = 0x1,
	I2C_CMD_MASTER = 0x2,
	I2C_CMD_READ = 0x4,
	I2C_CMD_WRITE = 0x8,
} i2c_cmd_t;

typedef struct{
	itask_queue_t master_cmds,
				  slave_cmds;
	ringbuf_t rx_buf;

	mutex_t mtx;
} i2c_int_data_t;

typedef struct{
	int (*configure)(i2c_cfg_t *cfg, bool int_en, void *data);

	i2c_state_t (*state)(bool wait, void *data);

	void (*start)(void *data);

	void (*slave_mode_set)(bool stop, bool int_en, void *data);
	void (*slave_read_write)(uint8_t remote, bool read, bool int_en, void *data);

	void (*byte_request)(bool ack, void *data);
	uint8_t (*byte_read)(void *data);
	void (*byte_write)(uint8_t c, bool ack, void *data);

	int_num_t int_num;

	void *data;
} i2c_primitives_t;


/* prototypes */
#ifdef CONFIG_I2C_PROTO_POLL
size_t i2c_master_read(uint8_t remote, uint8_t *buf, size_t n, i2c_primitives_t *prim);
size_t i2c_master_write(uint8_t remote, uint8_t *buf, size_t n, i2c_primitives_t *prim);
size_t i2c_slave_read(uint8_t *buf, size_t n, i2c_primitives_t *prim);
size_t i2c_slave_write(uint8_t *buf, size_t n, i2c_primitives_t *prim);
#endif // CONFIG_I2C_PROTO_POLL

#ifdef CONFIG_I2C_PROTO_INT
int i2c_int_data_init(i2c_int_data_t *data);
size_t i2c_int_cmd(i2c_cmd_t cmd, i2c_int_data_t *data, uint8_t remote, void *buf, size_t n, i2c_primitives_t *prim);
void i2c_int_hdlr(i2c_int_data_t *data, i2c_primitives_t *prim);
#endif // CONFIG_I2C_PROTO_INT


#endif // DRIVER_I2C_H
