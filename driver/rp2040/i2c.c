/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <kernel/driver.h>
#include <driver/i2c.h>


/* macros */
// register bits
#define CON_STOP_DET_IF_MASTER_ACTIVE			10
#define CON_RX_FIFO_FULL_HLD_CTRL				9
#define CON_TX_EMPTY_CTRL						8
#define CON_STOP_DET_IF_ADDRESSED				7
#define CON_IC_SLAVE_DISABLED					6
#define CON_IC_RESTART_EN						5
#define CON_IC_10BITADDR_MASTER					4
#define CON_IC_10BITADDR_SLAVE					3
#define CON_SPEED								1
#define CON_MASTER_MODE							0

#define ACK_GENERAL_CALL_ACK_GEN_CALL			0
#define SLV_DATA_NACK_ONLY_NACK					0

#define SDA_SETUP_SETUP							0

#define SDA_HOLD_SDA_RX_HOLD					16
#define SDA_HOLD_SDA_TX_HOLD					0

#define TAR_SPECIAL								11
#define TAR_GC_OR_START							10
#define TAR_IC_TAR								0
#define SAR_IC_SAR								0

#define ENABLE_TX_CMD_BLOCK						2
#define ENABLE_ABORT							1
#define ENABLE_ENABLE							0

#define ENABLE_STATUS_SLV_RX_DATA_LOST			2
#define ENABLE_STATUS_SLV_DISABLED_WHILE_BUSY	1
#define ENABLE_STATUS_IC_EN						0

#define STATUS_SLV_ACTIVITY						6
#define STATUS_MST_ACTIVITY						5
#define STATUS_RX_FIFO_FULL						4
#define STATUS_RX_FIFO_NOT_EMPTY				3
#define STATUS_TX_FIFO_EMPTY					2
#define STATUS_TX_FIFO_NOT_FULL					1
#define STATUS_ACTIVITY							0

#define DATA_CMD_FIRST_DATA_BYTE				11
#define DATA_CMD_RESTART						10
#define DATA_CMD_STOP							9
#define DATA_CMD_CMD							8
#define DATA_CMD_DAT							0

#define TXFLR_LEVEL								0
#define RXFLR_LEVEL								0
#define RX_TL_LEVEL								0
#define TX_TL_LEVEL								0

#define DMA_CR_TDMAE							1
#define DMA_CR_RDMAE							0
#define DMA_TDLR_DMATDL							0
#define DMA_RDLR_DMARDL							0

#define INT_RESTART_DET							12
#define INT_GEN_CALL							11
#define INT_START_DET							10
#define INT_STOP_DET							9
#define INT_ACTIVITY							8
#define INT_RX_DONE								7
#define INT_TX_ABRT								6
#define INT_RD_REQ								5
#define INT_TX_EMPTY							4
#define INT_TX_OVER								3
#define INT_RX_FULL								2
#define INT_RX_OVER								1
#define INT_RX_UNDER							0


/* types */
typedef struct{
	uint32_t volatile con,
					  tar,
					  sar,
					  padding0,
					  data_cmd,
					  ss_scl_hcnt,
					  ss_scl_lcnt,
					  fs_scl_hcnt,
					  fs_scl_lcnt,
					  padding1[2],
					  intr_stat,
					  intr_mask,
					  raw_intr_stat,
					  rx_tl,
					  tx_tl,
					  clr_intr,
					  clr_rx_under,
					  clr_rx_over,
					  clr_tx_over,
					  clr_rd_req,
					  clr_tx_abrt,
					  clr_rx_done,
					  clr_activity,
					  clr_stop_det,
					  clr_start_det,
					  clr_gen_call,
					  enable,
					  status,
					  txflr,
					  rxflr,
					  sda_hold,
					  tx_abrt_source,
					  slv_data_nack_only,
					  dma_cr,
					  dma_tdlr,
					  dma_rdlr,
					  sda_setup,
					  ack_gen_call,
					  enable_status,
					  fs_spklen,
					  padding2,
					  clr_resetart_det;
} i2c_regs_t;

typedef struct{
	uint8_t reset_id;
	i2c_regs_t *regs;

	// TODO is there a way to get the cfg aligned in the devtree
	// 		without moving reset_id to the top of the struct
	i2c_cfg_t cfg;
} dt_data_t;

typedef struct{
	dt_data_t *dtd;

	i2c_state_t state;
	i2c_mode_t mode;
	i2c_cmd_t cmd;
} dev_data_t;


#include <kernel/kprintf.h>
#include <sys/ctype.h>

/* local/static prototypes */
static int configure(i2c_cfg_t *cfg, void *hw);
static i2c_state_t state(void *hw);
static void start(void *hw);
static size_t ack(size_t remaining, void *hw);
static void idle(bool addressable, bool stop, void *hw);
static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw);
static size_t read(uint8_t *buf, size_t n, void *hw);
static size_t write(uint8_t *buf, size_t n, bool last, void *hw);
static void enable(bool en, i2c_regs_t *regs);
int configure_transfer(dev_data_t *i2c, uint8_t slave);
static size_t rx(uint8_t *buf, size_t n, bool blocking, i2c_regs_t *regs);
static size_t tx(uint8_t *buf, size_t n, i2c_cmd_t cmd, bool blocking, bool stop, i2c_regs_t *regs);

#include <sys/string.h>

/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	i2c_ops_t ops;
	i2c_t *itf;
	dev_data_t *i2c;


	i2c = kmalloc(sizeof(dev_data_t));

	if(i2c == 0x0)
		goto err_0;

	i2c->dtd = dtd;
	i2c->mode = I2C_SLAVE;
	i2c->cmd = I2C_READ;
	i2c->state = I2C_STATE_NONE;

	ops.configure = configure;
	ops.state = state;
	ops.start = start;
	ops.ack = ack;
	ops.idle = idle;
	ops.connect = connect;
	ops.read = read;
	ops.write = write;

	itf = i2c_create(&ops, &dtd->cfg, i2c);

	if(itf == 0x0)
		goto err_1;

	return itf;


err_1:
	kfree(i2c);

err_0:
	return 0x0;
}

driver_probe("rp2040,i2c", probe);

static int configure(i2c_cfg_t *cfg, void *hw){
	uint8_t speeds[] = { 1, 2, 2, 3, 1, 1 };
	dev_data_t *i2c = (dev_data_t*)hw;
	dt_data_t *dtd = i2c->dtd;
	i2c_regs_t *regs = dtd->regs;
	uint32_t ic_clk_hz;
	i2c_speed_t speed;
	i2c_timing_t *timings;


	speed = i2c_speed(cfg->clock_khz);
	timings = i2c_timing(speed);
	ic_clk_hz = RP2040_SYSTEM_CLOCK_HZ;

	if(speed == I2C_SPD_INVAL)
		return_errno(E_LIMIT);

	// NOTE
	// 	The i2c hardware is an implementation of the Synopsys dw_apb_i2c which
	// 	supports speeds up to high speed mode (3.4Mbps). However, the rp2040
	// 	maximum speed is fast mode plus (1Mbps) and the following implementation
	// 	needs to be updated if it shall be used for other implementations of the
	// 	dw_apb_i2c.
	rp2040_resets_release(0x1 << dtd->reset_id);

	enable(false, regs);

	regs->con = (speeds[speed] << CON_SPEED)
			  // start in slave mode
			  | (0x0 << CON_IC_SLAVE_DISABLED)
			  | (0x0 << CON_MASTER_MODE)
			  // NOTE so far only 7-bit addresses are supported
			  | (0x0 << CON_IC_10BITADDR_MASTER)
			  | (0x0 << CON_IC_10BITADDR_SLAVE)
			  | (0x1 << CON_RX_FIFO_FULL_HLD_CTRL)
			  | (0x1 << CON_TX_EMPTY_CTRL)
			  |	(0x1 << CON_STOP_DET_IF_ADDRESSED)	// TODO check
			  | ((speed >= I2C_SPD_HIGH) << CON_IC_RESTART_EN)
			  ;
	regs->slv_data_nack_only = (0x0 << SLV_DATA_NACK_ONLY_NACK);
	regs->ack_gen_call = (cfg->bcast_en << ACK_GENERAL_CALL_ACK_GEN_CALL);

	// slave address
	regs->sar = cfg->addr << SAR_IC_SAR;

	// interrupts
	regs->intr_mask = 0x0;	// TODO enable interrupts
	regs->rx_tl = 0;		// TODO update (max is implementation defined - should be 16 - but set to 0xff since hardware sets max then)
	regs->tx_tl = 0;		// TODO update

	// dma
	regs->dma_cr = (0x1 << DMA_CR_RDMAE) | (0x1 << DMA_CR_TDMAE);
	regs->dma_rdlr = 0x0;
	regs->dma_tdlr = 0x0;

	// timing
	// TODO check constraints for values in text and register description
	// 		check individual register docu
	regs->fs_spklen = (timings->spike_len_ns * (ic_clk_hz / 1000)) / 1000000;
	regs->fs_scl_lcnt = ((timings->scl_low_ns + timings->scl_fall_ns) * (ic_clk_hz / 1000)) / 1000000;
	regs->fs_scl_hcnt = (ic_clk_hz / 1000) / cfg->clock_khz - regs->fs_scl_lcnt;
	regs->ss_scl_lcnt = regs->fs_scl_lcnt;	// TODO are there different requirments to the SS than to the FS CNT registers
	regs->ss_scl_hcnt = regs->fs_scl_hcnt;
	regs->sda_hold = (timings->data_hold_ns * (ic_clk_hz / 1000)) / 1000000;
	regs->sda_setup = (timings->data_setup_ns * (ic_clk_hz / 1000)) / 1000000;

	enable(true, regs);

	DEBUG("i2c config: addr=%u\n", cfg->addr);
	DEBUG("  speed=%u spklen=%u ss_hcnt=%u ss_lcnt=%u fs_hcnt=%u fs_lcnt=%u, setup=%u, hold=%u\n",
		speed,
		regs->fs_spklen,
		regs->ss_scl_hcnt,
		regs->ss_scl_lcnt,
		regs->fs_scl_hcnt,
		regs->fs_scl_lcnt,
		regs->sda_setup,
		regs->sda_hold
	);

	return 0;
}

static i2c_state_t state(void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;
	i2c_regs_t *regs = i2c->dtd->regs;


	// TODO update based on hardware state
	switch(i2c->state){
	case I2C_STATE_MST_SLAW_ACK:
		if(regs->txflr == 0)
			return I2C_STATE_MST_SLAW_DATA_ACK;

		return I2C_STATE_NONE;

	case I2C_STATE_MST_SLAR_DATA_ACK:
		if(regs->status & (0x1 << STATUS_RX_FIFO_NOT_EMPTY))
			return I2C_STATE_MST_SLAR_DATA_ACK;

		return I2C_STATE_NONE;

	case I2C_STATE_SLA_SLAW_DATA_ACK:
		if(regs->rxflr > 0)
			return I2C_STATE_SLA_SLAW_DATA_ACK;

		// TODO should the following be used or not
		// 		scenario: slave wants to read more than the master sends
		// 		If it is disabled the slave will wait until all bytes that it expects
		// 		are received. If enabled the slave will only wait til the stop signal
		// 		and not wait for more bytes.
		if(regs->raw_intr_stat & (1 << INT_STOP_DET)){
			(void)regs->clr_stop_det;
			return I2C_STATE_SLA_SLAW_DATA_NACK;
		}

		return I2C_STATE_NONE;

	case I2C_STATE_SLA_SLAR_MATCH:
//		if(regs->raw_intr_stat & (0x1 << INT_TX_EMPTY))
		if(regs->txflr == 0)
			return I2C_STATE_SLA_SLAR_DATA_ACK;

		return I2C_STATE_NONE;

	case I2C_STATE_NONE:
		if(regs->raw_intr_stat & (0x1 << INT_RD_REQ)){
			i2c->state = I2C_STATE_SLA_SLAR_MATCH;

			return I2C_STATE_SLA_SLAR_MATCH;
		}

		if(i2c->mode == I2C_SLAVE && regs->rxflr > 0)
			return I2C_STATE_SLA_SLAW_MATCH;

		// fall through
	default:
		return i2c->state;
	}
}

static void start(void *hw){
	((dev_data_t*)hw)->state = I2C_STATE_MST_START;
}

static size_t ack(size_t remaining, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	/* master read */
	if(i2c->state == I2C_STATE_MST_SLAR_ACK || i2c->state == I2C_STATE_MST_SLAR_DATA_ACK){
		DEBUG("ack: remaining=%zu stop=%u\n", remaining > 2 ? 2 : remaining, remaining <= 2);
		i2c->state = I2C_STATE_MST_SLAR_DATA_ACK;

		return tx(0x0, remaining > 2 ? 2 : remaining, I2C_READ, false, remaining <= 2, i2c->dtd->regs);
	}

	/* slave read */
	i2c->state = I2C_STATE_SLA_SLAW_DATA_ACK;

	if(remaining <= 1)
		i2c->state = I2C_STATE_SLA_SLAW_DATA_NACK;

	return remaining;
}

static void idle(bool addressable, bool stop, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	i2c->mode = I2C_SLAVE;
	i2c->cmd = I2C_READ;	// cmd doesn't // TODO enhance comment
	i2c->state = I2C_STATE_NONE;

	if(addressable){
		if(configure_transfer(i2c, 0) != 0)
			i2c->state = I2C_STATE_ERROR;
	}
}

static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	i2c->mode = I2C_MASTER;
	i2c->cmd = cmd;
	i2c->state = (cmd == I2C_READ) ? I2C_STATE_MST_SLAR_ACK : I2C_STATE_MST_SLAW_ACK;

	if(configure_transfer(i2c, slave) != 0)
		i2c->state = I2C_STATE_ERROR;
}

static size_t read(uint8_t *buf, size_t n, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	DEBUG("state: int=%#x state=%#x rx-lvl=%zu\n", i2c->dtd->regs->raw_intr_stat, i2c->dtd->regs->status, i2c->dtd->regs->rxflr);

	n = rx(buf, n > 2 ? 2 : n, false, i2c->dtd->regs);

	(void)i2c->dtd->regs->clr_rx_done;

	return n;
}

static size_t write(uint8_t *buf, size_t n, bool last, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	i2c->state = (i2c->mode == I2C_MASTER) ? I2C_STATE_MST_SLAW_ACK : I2C_STATE_SLA_SLAR_MATCH;

	// clear tx abort interrupt, which is raised if the tx fifo still contains data when
	// receiving a slave read request, required to flush the tx fifo
	(void)i2c->dtd->regs->clr_tx_abrt;

	n = tx(buf, n, I2C_WRITE, false, last, i2c->dtd->regs);

	// clear read request interrupt to allow multiple slave writes
	// if not cleared a subsequent slave write would immediately write its data even if no
	// master request is present, since this drivers state machine would immediately transition
	// to I2C_STATE_SLA_SLAR_MATCH if the read request interrupt is still asserted
	(void)i2c->dtd->regs->clr_rd_req;

	return n;
}

static size_t tx(uint8_t *buf, size_t n, i2c_cmd_t cmd, bool blocking, bool stop, i2c_regs_t *regs){
	for(size_t i=0; i<n; i++){
		while((regs->status & (0x1 << STATUS_TX_FIFO_NOT_FULL)) == 0){
			if(!blocking)
				return i;
		}

		regs->data_cmd = ((buf ? buf[i] : 0x0) << DATA_CMD_DAT)
					   | ((cmd == I2C_READ) << DATA_CMD_CMD)
					   | ((stop && i + 1 == n) << DATA_CMD_STOP)
					   ;

		if(buf)
			DEBUG("write: %hhx (%c)\n", buf[i], isprint(buf[i]) ? buf[i] : '.');
	}

	if(blocking)
		while((regs->status & (0x1 << STATUS_TX_FIFO_EMPTY)) == 0);

	return n;
}

static size_t rx(uint8_t *buf, size_t n, bool blocking, i2c_regs_t *regs){
	for(size_t i=0; i<n; i++){
//		while((regs->status & (0x1 << STATUS_RX_FIFO_NOT_EMPTY)) == 0){
		while(regs->rxflr == 0){
			if(!blocking)
				return i;
		}

		buf[i] = (uint8_t)(regs->data_cmd & 0xff);
	}

	return n;
}

static void enable(bool en, i2c_regs_t *regs){
	// TODO
	//	aborted transfer in the following scenario
	//		rpi: slave
	//		rpi: cat -n 1 -s 2
	//		avr: echo -n "1234"
	//		rpi: echo -n "12"
	//			-> abort
	if((regs->status & (0x1 << STATUS_TX_FIFO_EMPTY)) == 0 || (regs->status & (0x1 << STATUS_RX_FIFO_NOT_EMPTY)))
		WARN("abort incomplete operation (status=%#x)\n", regs->status);

	regs->enable = (en << ENABLE_ENABLE)
				 | (0x0 << ENABLE_TX_CMD_BLOCK)
				 | (0x1 << ENABLE_ABORT)
				 ;

	while((regs->enable_status & (0x1 << ENABLE_STATUS_IC_EN)) != en);
}

int configure_transfer(dev_data_t *i2c, uint8_t slave){
	bool gen_call = (slave == 0);
	i2c_regs_t *regs = i2c->dtd->regs;


	DEBUG("configure: mode=%s, slave=%u\n", (i2c->mode == I2C_MASTER) ? "master" : "slave", (i2c->mode == I2C_MASTER) ? slave : regs->sar);

	// clear interrupts
	// clear stop detected interrupt to allow successive slave reads
	// if the interrupt is not cleared and a slave read reads fewer data than available a stop condition
	// might be send before the next slave read is called. in that case only the first remaining byte would
	// be read from the rx fifo since the stop detection interrupt is still present
//	(void)regs->clr_stop_det;
//	(void)regs->clr_rd_req;
//	(void)regs->clr_intr;

	// do not change configuration if already in the desired this
	// this avoids issues of not being able to disable the hardware, e.g. if
	// the device is still addressed in slave mode, in that case the hardware
	// cannot be disabled, which would lead to enable() not being able to return
	// TODO check if the slave is also the same in master mode
	if(((i2c->mode == I2C_MASTER) && (regs->con & (0x1 << CON_MASTER_MODE))) || ((i2c->mode == I2C_SLAVE) && !(regs->con & (0x1 << CON_MASTER_MODE))))
		return 0;

	// TODO move this check to the common i2c driver
	if(gen_call && i2c->mode == I2C_MASTER && i2c->cmd == I2C_READ)
		return_errno(E_INVAL);

	if((regs->status & (0x1 << STATUS_SLV_ACTIVITY)) && (i2c->mode == I2C_MASTER))
		return_errno(E_INUSE);

	enable(false, regs);

	// TODO test general call
	regs->con &= ~((0x1 << CON_IC_SLAVE_DISABLED) | (0x1 << CON_MASTER_MODE));
	regs->con |= ((i2c->mode == I2C_MASTER) << CON_IC_SLAVE_DISABLED) | ((i2c->mode == I2C_MASTER) << CON_MASTER_MODE);

	regs->tar = (slave << TAR_IC_TAR)
			  | (gen_call << TAR_SPECIAL)
			  | (0x0 << TAR_GC_OR_START)
			  ;

	enable(true, regs);

	return 0;
}
