/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <driver/i2c.h>
#include <sys/blob.h>
#include <sys/errno.h>
#include <sys/i2c.h>
#include <sys/mutex.h>
#include <sys/stream.h>
#include <sys/string.h>
#include <sys/types.h>


/* local/static prototypes */
static size_t poll_cmd(i2c_t *i2c, i2c_dgram_t *dgram);
static size_t int_cmd(i2c_t *i2c, i2c_dgram_t *dgram);

static void int_hdlr(int_num_t num, void *payload);

static int dgram_hdlr(i2c_t *i2c, i2c_dgram_t *dgram);
static int master_hdlr(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state);
static int slave_hdlr(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state);

static int complete(i2c_t *i2c, errno_t errnum, bool stop);

static void dgram_init(i2c_dgram_t *dgram, i2c_mode_t mode, i2c_cmd_t cmd, int8_t slave, blob_t *bufs, size_t n);
static void *dgram_data(i2c_dgram_t *dgram);
static size_t dgram_chunk(i2c_dgram_t *dgram);
static void dgram_stage(i2c_dgram_t *dgram, size_t n);
static void dgram_commit(i2c_dgram_t *dgram, size_t n);
static bool dgram_last(i2c_dgram_t *dgram, size_t n);
static bool dgram_complete(i2c_dgram_t *dgram);

#ifdef BUILD_KERNEL_LOG_DEBUG
static char const *strstate(i2c_state_t state);
#endif // BUILD_KERNEL_LOG_DEBUG

/* global functions */
i2c_t *i2c_create(i2c_ops_t *ops, i2c_cfg_t *cfg, void *hw){
	i2c_t *i2c;


	if(i2c_address_reserved(cfg->addr))
		goto_errno(err_0, E_INVAL);

	/* allocated device */
	i2c = kcalloc(1, sizeof(i2c_t));

	if(i2c == 0x0)
		goto err_0;

	i2c->cfg = cfg;
	i2c->hw = hw;

	memcpy(&i2c->ops, ops, sizeof(i2c_ops_t));

	mutex_init(&i2c->mtx, MTX_NOINT);
	itask_queue_init(&i2c->cmds);

	/* configure device */
	if(ops->configure(i2c->cfg, i2c->hw) != 0)
		goto err_1;

	if(cfg->int_num && int_register(cfg->int_num, int_hdlr, i2c) != 0)
		goto err_1;

	ops->idle(false, false, hw);

	return i2c;


err_1:
	i2c_destroy(i2c);

err_0:
	return 0x0;
}

void i2c_destroy(i2c_t *i2c){
	if(i2c->cfg->int_num)
		int_release(i2c->cfg->int_num);

	itask_queue_destroy(&i2c->cmds);
	kfree(i2c);
}

size_t i2c_read(i2c_t *i2c, i2c_mode_t mode, uint8_t slave, void *buf, size_t n){
	return i2c_xfer(i2c, mode, I2C_READ, slave, BLOBS(BLOB(buf, n)), 1);
}

size_t i2c_write(i2c_t *i2c, i2c_mode_t mode, uint8_t slave, void *buf, size_t n){
	return i2c_xfer(i2c, mode, I2C_WRITE, slave, BLOBS(BLOB(buf, n)), 1);
}

size_t i2c_xfer(i2c_t *i2c, i2c_mode_t mode, i2c_cmd_t cmd, uint8_t slave, blob_t *bufs, size_t n){
	i2c_dgram_t dgram;


	if(i2c_address_reserved(slave)){
		set_errno(E_INVAL);

		return 0;
	}

	DEBUG("issue cmd: mode=%s, slave=%u, bufs=%zu\n", (mode == I2C_MASTER) ? "master" : "slave", slave, n);
	dgram_init(&dgram, mode, cmd, slave, bufs, n);

	return (i2c->cfg->int_num ? int_cmd(i2c, &dgram) : poll_cmd(i2c, &dgram));
}

bool i2c_address_reserved(uint8_t addr){
	// allow 0 as broadcast address
	if(addr == 0)
		return false;

	// according to the i2c specification addresses of the form 000 0xxx and 111 1xxx are reserved
	return ((addr & 0x78) == 0) || ((addr & 0x78) == 0x78);
}


/* local functions */
static size_t poll_cmd(i2c_t *i2c, i2c_dgram_t *dgram){
	i2c->dgram = 0x0;
	while(dgram_hdlr(i2c, dgram) > 0);

	return dgram->total;
}

static size_t int_cmd(i2c_t *i2c, i2c_dgram_t *dgram){
	if(itask_issue(&i2c->cmds, dgram, i2c->cfg->int_num) != 0)
		return 0;

	return dgram->total;
}

static void int_hdlr(int_num_t num, void *payload){
	i2c_t *i2c = (i2c_t*)payload;
	i2c_dgram_t *dgram;


	while(1){
		dgram = itask_query_payload(&i2c->cmds, 0x0);

		if(dgram == 0x0 || dgram_hdlr(i2c, dgram) > 0)
			break;

		itask_complete(&i2c->cmds, errno);
		i2c->dgram = 0x0;
	}
}

static int dgram_hdlr(i2c_t *i2c, i2c_dgram_t *dgram){
	int r;
	i2c_state_t state;



	mutex_lock(&i2c->mtx);

	state = (i2c->dgram != dgram) ? I2C_STATE_NEXT_CMD : i2c->ops.state(i2c->hw);
	i2c->dgram = dgram;

	// NOTE the error of handling a master-mode dgram while the hardware is in a slave state is not
	// 		checked here, since it is handled in the master_hdlr's default case, causing an io error
	r = (dgram->mode == I2C_MASTER) ? master_hdlr(i2c, dgram, state) : slave_hdlr(i2c, dgram, state);

	mutex_unlock(&i2c->mtx);

	return r;
}

static int master_hdlr(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state){
	i2c_ops_t *ops = &i2c->ops;
	size_t n;


	switch(state){
	/* master start */
	case I2C_STATE_MST_ARBLOST:
		DEBUG("arb-lost: state=%s\n", strstate(state));

		// fall through
	case I2C_STATE_NEXT_CMD:
		DEBUG("issue start\n");
		ops->start(i2c->hw);
		break;

	case I2C_STATE_MST_START:
		DEBUG("start: state=%s\n", strstate(state));
		ops->connect(dgram->cmd, dgram->slave, i2c->hw);
		break;

	case I2C_STATE_MST_START_NACK:
		DEBUG("nack: state=%s\n", strstate(state));
		return complete(i2c, E_NOCONN, true);


	/* master write */
	case I2C_STATE_MST_WR_DATA_ACK:
	case I2C_STATE_MST_WR_DATA_NACK:
		dgram_commit(dgram, ops->acked(dgram->staged, i2c->hw));

		DEBUG("(n)ack: state=%s\n", strstate(state));

		if(dgram_complete(dgram) || state == I2C_STATE_MST_WR_DATA_NACK)
			return complete(i2c, 0, true);

		if(dgram->staged)
			break;

		// fall through
	case I2C_STATE_MST_WR_ACK:
		n = dgram_chunk(dgram);
		DEBUG("sla-w: state=%s, n=%zu\n", strstate(state), n);

		dgram_stage(dgram, ops->write(dgram_data(dgram), n, dgram_last(dgram, n), i2c->hw));
		break;


	/* master read */
	case I2C_STATE_MST_RD_DATA_ACK:
	case I2C_STATE_MST_RD_DATA_NACK:
		n = ops->read(dgram_data(dgram), dgram->staged, i2c->hw);
		DEBUG("read: state=%s, n=%zu\n", strstate(state), n);

		dgram_commit(dgram, n);

		if(dgram_complete(dgram) || state == I2C_STATE_MST_RD_DATA_NACK)
			return complete(i2c, 0, true);

		if(dgram->staged)
			break;

		// fall through
	case I2C_STATE_MST_RD_ACK:
		dgram_stage(dgram, ops->ack(dgram_chunk(dgram), i2c->hw));
		DEBUG("sla-r: state=%s, staged=%zu\n", strstate(state), dgram->staged);
		break;


	/* no state change */
	case I2C_STATE_NONE:
		break;

	/* error */
	case I2C_STATE_ERROR:
	default:
		return complete(i2c, E_IO, true);
	}

	return 1;
}

static int slave_hdlr(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state){
	i2c_ops_t *ops = &i2c->ops;
	size_t n;


	switch(state){
	case I2C_STATE_NEXT_CMD:
		DEBUG("make addressable\n");
		i2c->ops.idle(true, false, i2c->hw);
		break;

	/* slave read */
	case I2C_STATE_SLA_RD_DATA_ACK:
	case I2C_STATE_SLA_RD_DATA_NACK:
		n = ops->read(dgram_data(dgram), dgram->staged, i2c->hw);
		DEBUG("read: state=%s, n=%zu\n", strstate(state), n);

		dgram_commit(dgram, n);

		if(dgram_complete(dgram) || state == I2C_STATE_SLA_RD_DATA_NACK)
			return complete(i2c, 0, false);

		if(dgram->staged)
			break;

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmission has beem finished
	case I2C_STATE_SLA_RD_MATCH:
		dgram_stage(dgram, ops->ack(dgram_chunk(dgram), i2c->hw));

		DEBUG("match: state=%s, staged=%zu\n", strstate(state), dgram->staged);
		break;

	case I2C_STATE_SLA_RD_STOP:
		DEBUG("stop: state=%s\n", strstate(state));
		return complete(i2c, 0, false);


	/* slave write */
	case I2C_STATE_SLA_WR_DATA_ACK:
	case I2C_STATE_SLA_WR_DATA_NACK:
		DEBUG("(n)ack: state=%s\n", strstate(state));

		dgram_commit(dgram, ops->acked(dgram->staged, i2c->hw));

		if(dgram_complete(dgram) || state == I2C_STATE_SLA_WR_DATA_NACK)
			return complete(i2c, 0, false);

		if(dgram->staged)
			break;

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmission has beem finished
	case I2C_STATE_SLA_WR_MATCH:
		n = dgram_chunk(dgram);
		DEBUG("write: state=%s, n=%zu\n", strstate(state), n);

		dgram_stage(dgram, ops->write(dgram_data(dgram), n, dgram_last(dgram, n), i2c->hw));
		break;


	/* no state change */
	case I2C_STATE_NONE:
		break;

	/* error */
	case I2C_STATE_ERROR:
	default:
		return complete(i2c, E_IO, true);
	}

	return 1;
}

static int complete(i2c_t *i2c, errno_t errnum, bool stop){
	errnum = errno ? errno : errnum;

	DEBUG("complete: status=%s, bytes=%zu\n", strerror(errnum), i2c->dgram->total);
	i2c->ops.idle(false, stop, i2c->hw);

	return_errno(errnum);
}

static void dgram_init(i2c_dgram_t *dgram, i2c_mode_t mode, i2c_cmd_t cmd, int8_t slave, blob_t *bufs, size_t n){
	dgram->mode = mode;
	dgram->cmd = cmd;
	dgram->slave = slave;

	dgram->bufs = bufs;
	dgram->nbuf = n;
	dgram->buf_lvl = 0;

	dgram->staged = 0;
	dgram->total = 0;
}

static void *dgram_data(i2c_dgram_t *dgram){
	return dgram->bufs->buf + dgram->buf_lvl;
}

static size_t dgram_chunk(i2c_dgram_t *dgram){
	if(dgram->nbuf == 0)
		return 0;

	if(dgram->buf_lvl >= dgram->bufs->len){
		dgram->bufs++;
		dgram->nbuf--;
		dgram->buf_lvl = 0;
	}

	return dgram->bufs->len - dgram->buf_lvl;
}

static void dgram_stage(i2c_dgram_t *dgram, size_t n){
	dgram->staged = n;
}

static void dgram_commit(i2c_dgram_t *dgram, size_t n){
	dgram->buf_lvl += n;
	dgram->total += n;
	dgram->staged -= n;
}

static bool dgram_last(i2c_dgram_t *dgram, size_t n){
	return (dgram->buf_lvl + n >= dgram->bufs->len) && (dgram->nbuf == 1);
}

static bool dgram_complete(i2c_dgram_t *dgram){
	return dgram_last(dgram, 0);
}

#ifdef BUILD_KERNEL_LOG_DEBUG
static char const *strstate(i2c_state_t state){
	static char s[5];


	switch(state){
	case I2C_STATE_SPECIAL:				return "special";
	case I2C_STATE_MASTER:				return "mst";
	case I2C_STATE_SLAVE:				return "sla";
	case I2C_STATE_NEXT_CMD:			return "next";
	case I2C_STATE_ERROR:				return "error";
	case I2C_STATE_NONE:				return "none";
	case I2C_STATE_MST_START:			return "mst-start";
	case I2C_STATE_MST_START_NACK:		return "mst-start-nack";
	case I2C_STATE_MST_WR_ACK:			return "mst-wr-ack";
	case I2C_STATE_MST_WR_DATA_ACK:		return "mst-wr-data-ack";
	case I2C_STATE_MST_WR_DATA_NACK:	return "mst-wr-data-nack";
	case I2C_STATE_MST_RD_ACK:			return "mst-rd-ack";
	case I2C_STATE_MST_RD_DATA_ACK:		return "mst-rd-data-ack";
	case I2C_STATE_MST_RD_DATA_NACK:	return "mst-rd-data-nack";
	case I2C_STATE_MST_ARBLOST:			return "mst-arblost";
	case I2C_STATE_SLA_RD_MATCH:		return "sla-rd-match";
	case I2C_STATE_SLA_RD_DATA_ACK:		return "sla-rd-data-ack";
	case I2C_STATE_SLA_RD_DATA_NACK:	return "sla-rd-data-nack";
	case I2C_STATE_SLA_RD_STOP:			return "sla-rd-stop";
	case I2C_STATE_SLA_WR_MATCH:		return "sla-wr-match";
	case I2C_STATE_SLA_WR_DATA_ACK:		return "sla-wr-data-ack";
	case I2C_STATE_SLA_WR_DATA_NACK:	return "sla-wr-data-nack";

	default:
		snprintf(s, sizeof(s), "%hhx", state);
		return s;
	}
}
#endif // BUILD_KERNEL_LOG_DEBUG
