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


/* types */
typedef struct{
	i2c_mode_t mode;
	i2c_cmd_t cmd;
	uint8_t slave;

	blob_t *blobs;
	size_t nblobs;

	size_t n,
		   incomplete;
} i2c_dgram_t;


/* local/static prototypes */
static int poll_cmd(i2c_t *i2c, i2c_dgram_t *dgram);
static int int_cmd(i2c_t *i2c, i2c_dgram_t *dgram);

static void int_hdlr(int_num_t num, void *payload);

static int int_master(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state);
static int int_slave(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state);

static int complete(i2c_t *i2c, errno_t errnum, bool stop);

#ifdef BUILD_KERNEL_LOG_DEBUG
static char const *strstate(i2c_state_t state);
#endif // BUILD_KERNEL_LOG_DEBUG

#include <sys/ctype.h>


/* global functions */
i2c_t *i2c_create(i2c_ops_t *ops, i2c_cfg_t *cfg, void *hw){
	i2c_t *i2c;


	/* allocated device */
	i2c = kcalloc(1, sizeof(i2c_t));

	if(i2c == 0x0)
		goto err_0;

	i2c->cfg = cfg;
	i2c->hw = hw;

	memcpy(&i2c->ops, ops, sizeof(i2c_ops_t));

	mutex_init(&i2c->mtx, MTX_NOINT);
	itask_queue_init(&i2c->master_cmds);
	itask_queue_init(&i2c->slave_cmds);

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

	itask_queue_destroy(&i2c->master_cmds);
	itask_queue_destroy(&i2c->slave_cmds);

	kfree(i2c);
}

int i2c_read(i2c_t *i2c, i2c_mode_t mode, uint8_t slave, void *buf, size_t n){
	return i2c_xfer(i2c, mode, I2C_READ, slave, BLOBS(BLOB(buf, n)), 1);
}

int i2c_write(i2c_t *i2c, i2c_mode_t mode, uint8_t slave, void *buf, size_t n){
	return i2c_xfer(i2c, mode, I2C_WRITE, slave, BLOBS(BLOB(buf, n)), 1);
}

int i2c_xfer(i2c_t *i2c, i2c_mode_t mode, i2c_cmd_t cmd, uint8_t slave, blob_t *bufs, size_t n){
	i2c_dgram_t dgram;


	dgram.mode = mode;
	dgram.cmd = cmd;
	dgram.slave = slave;
	dgram.blobs = bufs;
	dgram.nblobs = n;
	dgram.n = 0;
	dgram.incomplete = 0;

	DEBUG("issue cmd: mode=%s, slave=%u, blobs=%zu\n", (cmd & I2C_MASTER) ? "master" : "slave", slave, n);

	return (i2c->cfg->int_num ? int_cmd(i2c, &dgram) : poll_cmd(i2c, &dgram));
}

static int poll_cmd(i2c_t *i2c, i2c_dgram_t *dgram){
	int r;
	i2c_state_t state;
	int (*op)(i2c_t *, i2c_dgram_t *, i2c_state_t);


	state = I2C_STATE_NEXT_CMD;
	op = (dgram->mode == I2C_SLAVE) ? int_slave : int_master;

	while(1){
		mutex_lock(&i2c->mtx);

		r = op(i2c, dgram, state);
		state = i2c->ops.state(i2c->hw);

		mutex_unlock(&i2c->mtx);

		if(r <= 0)
			return_errno(-r);
	}
}

static int int_cmd(i2c_t *i2c, i2c_dgram_t *dgram){
	return itask_issue(
		(dgram->mode == I2C_MASTER) ? &i2c->master_cmds : &i2c->slave_cmds,
		dgram,
		i2c->cfg->int_num
	);
}

static void int_hdlr(int_num_t num, void *payload){
	i2c_t *i2c = (i2c_t*)payload;
	int r;
	bool master_cmd;
	i2c_state_t state;
	i2c_dgram_t *dgram;
	itask_queue_t *cmds;


	mutex_lock(&i2c->mtx);

	/* identify dgram */
	state = i2c->ops.state(i2c->hw);

	master_cmd = !(state & I2C_STATE_BIT_SLAVE);
	cmds = master_cmd ? &i2c->master_cmds : &i2c->slave_cmds;
	dgram = itask_query_payload(cmds, 0x0);

	if(dgram == 0x0)
		goto unlock;

	/* perform state action */
	// ensure master commands are started
	if(state == I2C_STATE_NONE)
		state = I2C_STATE_NEXT_CMD;

	r = master_cmd ? int_master(i2c, dgram, state) : int_slave(i2c, dgram, state);

	if(r > 0)
		goto unlock;

	/* transfer complete */
	itask_complete(cmds, -r);

	if(itask_query_payload(&i2c->master_cmds, 0x0) || itask_query_payload(&i2c->slave_cmds, 0x0))
		int_foretell(i2c->cfg->int_num);

unlock:
	mutex_unlock(&i2c->mtx);
}

static int int_master(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state){
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

	case I2C_STATE_MST_SLAW_NACK:
	case I2C_STATE_MST_SLAR_NACK:
		DEBUG("nack: state=%s\n", strstate(state));
		return complete(i2c, E_NOCONN, true);


	/* master write */
	case I2C_STATE_MST_SLAW_DATA_ACK:
		dgram->n += dgram->incomplete;

		// fall through
	case I2C_STATE_MST_SLAW_DATA_NACK:
		DEBUG("(n)ack: state=%s\n", strstate(state));

		if((dgram->n >= dgram->blobs->len && dgram->nblobs == 1) || state == I2C_STATE_MST_SLAW_DATA_NACK)
			return complete(i2c, 0, true);

		if(dgram->n >= dgram->blobs->len){
			dgram->blobs++;
			dgram->nblobs--;
			dgram->n = 0;
		}

		// fall through
	case I2C_STATE_MST_SLAW_ACK:
		n = dgram->blobs->len - dgram->n;
		DEBUG("sla-w: state=%s, n=%zu\n", strstate(state), n);

		// TODO move the condition for 'last' in a function, it is also used to detect if the
		// 		a dgram has been transfered completely
		dgram->incomplete = ops->write(dgram->blobs->buf + dgram->n, n, (dgram->n + n >= dgram->blobs->len && dgram->nblobs == 1), i2c->hw);
		break;


	/* master read */
	case I2C_STATE_MST_SLAR_DATA_ACK:
	case I2C_STATE_MST_SLAR_DATA_NACK:
		n = ops->read(dgram->blobs->buf + dgram->n, dgram->blobs->len - dgram->n, i2c->hw);
		DEBUG("read: state=%s, n=%zu\n", strstate(state), n);

		for(size_t i=0; i<n; i++){
			char c = ((char*)dgram->blobs->buf)[dgram->n + i];
			DEBUG("read: %hhx (%c)\n", c, isprint(c) ? c : '.');
		}

		dgram->n += n;
		dgram->incomplete -= n;

		if(dgram->incomplete)
			break;

		if((dgram->n >= dgram->blobs->len && dgram->nblobs == 1) || state == I2C_STATE_MST_SLAR_DATA_NACK)
			return complete(i2c, 0, true);

		if(dgram->n >= dgram->blobs->len){
			dgram->blobs++;
			dgram->nblobs--;
			dgram->n = 0;
		}

		// fall through
	case I2C_STATE_MST_SLAR_ACK:
		n = dgram->blobs->len - dgram->n;
		dgram->incomplete = ops->ack(n, i2c->hw);

		DEBUG("sla-r: state=%s, n=%zu, incomplete=%zu\n", strstate(state), n, dgram->incomplete);
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

static int int_slave(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state){
	i2c_ops_t *ops = &i2c->ops;
	size_t n;

	// TODO
	// 	it seems slave ops cannot read or write multiple blobs

	switch(state){
	case I2C_STATE_NEXT_CMD:
		DEBUG("make addressable\n");
		i2c->ops.idle(true, false, i2c->hw);
		break;

	/* slave read */
	case I2C_STATE_SLA_SLAW_DATA_ACK:
	case I2C_STATE_SLA_SLAW_DATA_NACK:
		n = ops->read(dgram->blobs->buf + dgram->n, dgram->blobs->len - dgram->n, i2c->hw);

		DEBUG("read: state=%s, n=%zu\n", strstate(state), n);

		for(size_t i=0; i<n; i++){
			char c = ((char*)dgram->blobs->buf)[dgram->n + i];
			DEBUG("read: %hhx (%c)\n", c, isprint(c) ? c : '.');
		}

		dgram->n += n;

		if(state == I2C_STATE_SLA_SLAW_DATA_NACK)
			return complete(i2c, 0, false);

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmission has beem finished
	case I2C_STATE_SLA_SLAW_MATCH:
		n = dgram->blobs->len - dgram->n;
		DEBUG("match: state=%s, n=%zu\n", strstate(state), n);
		ops->ack(n, i2c->hw);	// TODO ack now has a return value, check also other functions with changed return values
		break;


	/* slave write */
	case I2C_STATE_SLA_SLAR_DATA_ACK:
	case I2C_STATE_SLA_SLAR_DATA_NACK:
		DEBUG("(n)ack: state=%s\n", strstate(state));

		if(dgram->n >= dgram->blobs->len){
			itask_complete(&i2c->slave_cmds, 0);
			dgram = itask_query_payload(&i2c->slave_cmds, 0x0);
		}

		if(state == I2C_STATE_SLA_SLAR_DATA_NACK || dgram == 0x0)
			return complete(i2c, 0, false);

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmission has beem finished
	case I2C_STATE_SLA_SLAR_MATCH:
		n = dgram->blobs->len - dgram->n;
		DEBUG("write: state=%s, n=%zu\n", strstate(state), n);

		dgram->n += ops->write(dgram->blobs->buf + dgram->n, n, (dgram->blobs->len - dgram->n - n == 0), i2c->hw);
		break;

	case I2C_STATE_SLA_SLAW_STOP:
		DEBUG("stop: state=%s\n", strstate(state));
		return complete(i2c, 0, false);


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
	DEBUG("complete: %s, %sstop\n", strerror(errnum), (stop ? "" : "no "));
	i2c->ops.idle(false, stop, i2c->hw);

	return -errnum;
}

#ifdef BUILD_KERNEL_LOG_DEBUG
static char const *strstate(i2c_state_t state){
	static char s[5];


	switch(state){
	case I2C_STATE_BIT_SPECIAL:				return "special-bit";
	case I2C_STATE_BIT_MASTER:				return "mst-bit";
	case I2C_STATE_BIT_SLAVE:				return "sla-bit";
	case I2C_STATE_NEXT_CMD:				return "next";
	case I2C_STATE_ERROR:					return "error";
	case I2C_STATE_NONE:					return "none";
	case I2C_STATE_MST_START:				return "mst-start";
	case I2C_STATE_MST_SLAW_ACK:			return "mst-wr-ack";
	case I2C_STATE_MST_SLAW_NACK:			return "mst-wr-nack";
	case I2C_STATE_MST_SLAW_DATA_ACK:		return "mst-wr-data-ack";
	case I2C_STATE_MST_SLAW_DATA_NACK:		return "mst-wr-data-nack";
	case I2C_STATE_MST_SLAR_ACK:			return "mst-rd-ack";
	case I2C_STATE_MST_SLAR_NACK:			return "mst-rd-nack";
	case I2C_STATE_MST_SLAR_DATA_ACK:		return "mst-rd-data-ack";
	case I2C_STATE_MST_SLAR_DATA_NACK:		return "mst-rd-data-nack";
	case I2C_STATE_MST_ARBLOST:				return "mst-arblost";
	case I2C_STATE_SLA_SLAW_MATCH:			return "sla-wr-match";
	case I2C_STATE_SLA_SLAR_MATCH:			return "sla-rd-match";
	case I2C_STATE_SLA_SLAW_DATA_ACK:		return "sla-wr-data-ack";
	case I2C_STATE_SLA_SLAW_DATA_NACK:		return "sla-wr-data-nack";
	case I2C_STATE_SLA_SLAW_STOP:			return "sla-wr-stop";
	case I2C_STATE_SLA_SLAR_DATA_ACK:		return "sla-rd-data-ack";
	case I2C_STATE_SLA_SLAR_DATA_NACK:		return "sla-rd-data-nack";

	default:
		snprintf(s, sizeof(s), "%hhx", state);
		return s;
	}
}
#endif // BUILD_KERNEL_LOG_DEBUG
