/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H


#include <kernel/inttask.h>
#include <sys/blob.h>
#include <sys/i2c.h>
#include <sys/mutex.h>
#include <sys/types.h>


/* incomplete types */
struct i2c_t;


/* types */
typedef enum{
	I2C_READ = 1,
	I2C_WRITE
} i2c_cmd_t;

typedef enum{
	I2C_SPD_STD = 0,
	I2C_SPD_FAST,
	I2C_SPD_FASTPLUS,
	I2C_SPD_HIGH,
	I2C_SPD_ULTRA,
	I2C_SPD_INVAL
} i2c_speed_t;

typedef enum{
	// state sections bits
	I2C_STATE_BIT_SPECIAL = 0x20,
	I2C_STATE_BIT_MASTER = 0x40,
	I2C_STATE_BIT_SLAVE = 0x80,

	// special states
	I2C_STATE_NEXT_CMD = I2C_STATE_BIT_SPECIAL | 0x01,
	I2C_STATE_ERROR,
	I2C_STATE_NONE,

	// master mode states
	I2C_STATE_MST_START = I2C_STATE_BIT_MASTER | 0x01,

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
	I2C_STATE_SLA_SLAW_DATA_ACK,
	I2C_STATE_SLA_SLAW_DATA_NACK,
	I2C_STATE_SLA_SLAW_STOP,

	I2C_STATE_SLA_SLAR_MATCH,
	I2C_STATE_SLA_SLAR_DATA_ACK,
	I2C_STATE_SLA_SLAR_DATA_NACK,
} i2c_state_t;

typedef struct{
	uint8_t spike_len_ns;
	uint8_t data_setup_ns;
	uint16_t scl_fall_ns;
	uint16_t data_hold_ns;
	uint16_t scl_low_ns;
	uint16_t scl_high_linux_ns;
} i2c_timing_t;

typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint16_t clock_khz;

	uint8_t bcast_en;
	uint8_t addr;

	uint8_t int_num;
} i2c_cfg_t;

typedef struct{
	/* primitives */
	int (*configure)(i2c_cfg_t *cfg, void *hw);

	i2c_state_t (*state)(void *hw);
	void (*start)(void *hw);
	size_t (*ack)(size_t remaining, void *hw);

	void (*idle)(bool addressable, bool stop, void *hw);
	void (*connect)(i2c_cmd_t cmd, uint8_t slave, void *hw);

	size_t (*read)(uint8_t *buf, size_t n, void *hw);
	size_t (*write)(uint8_t *buf, size_t n, bool last, void *hw);
} i2c_ops_t;

typedef struct i2c_t{
	i2c_cfg_t *cfg;
	i2c_ops_t ops;
	void *hw;

	itask_queue_t master_cmds,
				  slave_cmds;

	mutex_t mtx;
} i2c_t;


/* prototypes */
i2c_t *i2c_create(i2c_ops_t *ops, i2c_cfg_t *cfg, void *hw);
void i2c_destroy(i2c_t *i2c);

int i2c_read(i2c_t *i2c, i2c_mode_t mode, uint8_t slave, void *buf, size_t n);
int i2c_write(i2c_t *i2c, i2c_mode_t mode, uint8_t slave, void *buf, size_t n);
int i2c_xfer(i2c_t *i2c, i2c_mode_t mode, i2c_cmd_t cmd, uint8_t slave, blob_t *bufs, size_t n);

i2c_speed_t i2c_speed(uint16_t clock_khz);
i2c_timing_t *i2c_timing(i2c_speed_t speed);
bool i2c_address_reserved(uint8_t addr);


#endif // DRIVER_I2C_H
