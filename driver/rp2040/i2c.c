/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <kernel/driver.h>
#include <driver/i2c.h>
#include <sys/math.h>
#include <sys/register.h>


/* macros */
// hardware configuration
#define FIFO_DEPTH								16

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

#define TAR_SPECIAL								11
#define TAR_GC_OR_START							10
#define TAR_IC_TAR								0
#define SAR_IC_SAR								0

#define ENABLE_TX_CMD_BLOCK						2
#define ENABLE_ABORT							1
#define ENABLE_ENABLE							0

#define SDA_SETUP_SETUP							0

#define SDA_HOLD_SDA_RX_HOLD					16
#define SDA_HOLD_SDA_TX_HOLD					0

#define DMA_CR_TDMAE							1
#define DMA_CR_RDMAE							0

#define DATA_CMD_FIRST_DATA_BYTE				11
#define DATA_CMD_RESTART						10
#define DATA_CMD_STOP							9
#define DATA_CMD_CMD							8
#define DATA_CMD_DAT							0

#define STATUS_SLV_ACTIVITY						6
#define STATUS_MST_ACTIVITY						5
#define STATUS_RX_FIFO_FULL						4
#define STATUS_RX_FIFO_NOT_EMPTY				3
#define STATUS_TX_FIFO_EMPTY					2
#define STATUS_TX_FIFO_NOT_FULL					1
#define STATUS_ACTIVITY							0

#define ENABLE_STATUS_SLV_RX_DATA_LOST			2
#define ENABLE_STATUS_SLV_DISABLED_WHILE_BUSY	1
#define ENABLE_STATUS_IC_EN						0

#define ABRT_SRC_TX_FLUSH_CNT					23
#define ABRT_SRC_10ADDR2_NOACK					2
#define ABRT_SRC_10ADDR1_NOACK					1
#define ABRT_SRC_7B_ADDR_NOACK					0

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
	i2c_cfg_t cfg;
} dt_data_t;

typedef struct{
	dt_data_t *dtd;

	i2c_state_t state;
	i2c_mode_t mode;
} dev_data_t;


/* local/static prototypes */
static int configure(i2c_cfg_t *cfg, void *hw);
static i2c_state_t state(void *hw);
static void start(void *hw);
static size_t ack(size_t remaining, void *hw);
static size_t acked(size_t staged, void *hw);
static void idle(bool addressable, bool stop, void *hw);
static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw);
static size_t read(uint8_t *buf, size_t n, void *hw);
static size_t write(uint8_t *buf, size_t n, bool last, void *hw);

static int set_mode(dev_data_t *i2c, uint8_t slave, bool addressable);
static void reset(dev_data_t *i2c);
static void enable(bool en, dev_data_t *i2c);
static size_t tx(uint8_t *buf, size_t n, i2c_cmd_t cmd, bool stop, dev_data_t *i2c);
static size_t rx(uint8_t *buf, size_t n, dev_data_t *i2c);


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
	i2c->state = I2C_STATE_NONE;

	ops.configure = configure;
	ops.state = state;
	ops.start = start;
	ops.ack = ack;
	ops.acked = acked;
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
	uint32_t ic_clk_khz;
	i2c_speed_t speed;
	i2c_timing_t *timings;


	speed = i2c_speed(cfg->clock_khz);

	if(speed == I2C_SPD_INVAL)
		return_errno(E_LIMIT);

	rp2040_resets_halt(0x1 << dtd->reset_id);
	rp2040_resets_release(0x1 << dtd->reset_id);

	enable(false, i2c);

	regs->con = (speeds[speed] << CON_SPEED)
			  | (0x1 << CON_IC_SLAVE_DISABLED)
			  | (0x0 << CON_MASTER_MODE)
			  // NOTE so far only 7-bit addresses are supported
			  | (0x0 << CON_IC_10BITADDR_MASTER)
			  | (0x0 << CON_IC_10BITADDR_SLAVE)
			  | (0x1 << CON_RX_FIFO_FULL_HLD_CTRL)
			  | (0x1 << CON_TX_EMPTY_CTRL)
			  |	(0x1 << CON_STOP_DET_IF_ADDRESSED)
			  | ((speed >= I2C_SPD_HIGH) << CON_IC_RESTART_EN)
			  ;

	regs->slv_data_nack_only = (0x0 << SLV_DATA_NACK_ONLY_NACK);
	regs->ack_gen_call = (cfg->bcast_en << ACK_GENERAL_CALL_ACK_GEN_CALL);
	regs->rx_tl = 1;
	regs->tx_tl = 0;

	/* slave address */
	regs->sar = cfg->addr << SAR_IC_SAR;

	/* interrupts */
	regs->intr_mask = 0x0;

	if(cfg->int_num){
		regs->intr_mask = (0x1 << INT_ACTIVITY);
		av6m_nvic_int_enable(cfg->int_num);
	}

	/* dma */
	regs->dma_cr = (0x1 << DMA_CR_RDMAE) | (0x1 << DMA_CR_TDMAE);
	regs->dma_rdlr = 0x0;
	regs->dma_tdlr = 0x0;

	/* timing */
	// NOTE
	// 	The i2c hardware is an implementation of the Synopsys dw_apb_i2c which
	// 	supports speeds up to high speed mode (3.4Mbps). However, the rp2040
	// 	maximum speed is fast mode plus (1Mbps) and the following implementation
	// 	needs to be updated if it shall be used for other implementations of the
	// 	dw_apb_i2c.
	timings = i2c_timing(speed);
	ic_clk_khz = RP2040_SYSTEM_CLOCK_HZ / 1000;

	regs->fs_spklen = (timings->spike_len_ns * ic_clk_khz) / 1000000;

	// the datasheet min value of 8 is enforced by hardware
	regs->fs_scl_lcnt = ((timings->scl_low_ns + timings->scl_fall_ns) * ic_clk_khz) / 1000000;
	regs->fs_scl_lcnt = MAX(regs->fs_spklen + 7, regs->fs_scl_lcnt);
	regs->ss_scl_lcnt = regs->fs_scl_lcnt;

	// the datasheet min value of 6 is enforced by hardware
	regs->fs_scl_hcnt = ic_clk_khz / cfg->clock_khz - regs->fs_scl_lcnt;
	regs->fs_scl_hcnt = MAX(regs->fs_spklen + 5, regs->fs_scl_hcnt);
	regs->ss_scl_hcnt = regs->fs_scl_hcnt;

	regs->sda_setup = MAX((uint32_t)2, (((timings->data_setup_ns * ic_clk_khz) / 1000000) + 1)) << SDA_SETUP_SETUP;
	regs->sda_hold = (((timings->data_hold_ns * ic_clk_khz) / 1000000) << SDA_HOLD_SDA_RX_HOLD)
				   | (((timings->data_hold_ns * ic_clk_khz) / 1000000) << SDA_HOLD_SDA_TX_HOLD)
				   ;

	enable(true, i2c);

	return 0;
}

static i2c_state_t state(void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;
	i2c_regs_t *regs = i2c->dtd->regs;


	(void)regs->clr_activity;

	/* abort conditions */
	// do not clear tx-abort interrupt to allow detecting it in subsequently called
	// i2c protocol functions, in particular acked()
	if(regs->raw_intr_stat & (0x1 << INT_TX_ABRT)){
		if(regs->tx_abrt_source & ((0x1 << ABRT_SRC_7B_ADDR_NOACK) | (0x1 << ABRT_SRC_10ADDR1_NOACK) | (0x1 << ABRT_SRC_10ADDR2_NOACK)))
			return I2C_STATE_MST_START_NACK;

		switch(i2c->state){
		case I2C_STATE_MST_WR_DATA_ACK:	return I2C_STATE_MST_WR_DATA_NACK;
		case I2C_STATE_MST_RD_DATA_ACK:	return I2C_STATE_MST_RD_DATA_NACK;
		case I2C_STATE_SLA_WR_DATA_ACK:	return I2C_STATE_SLA_WR_DATA_NACK;
		default:						break;
		}
	}

	/* state transitions */
	switch(i2c->state){
	case I2C_STATE_MST_WR_DATA_ACK:
		if(regs->txflr == 0)
			return I2C_STATE_MST_WR_DATA_ACK;

		return I2C_STATE_NONE;

	case I2C_STATE_MST_RD_DATA_ACK:
		if(regs->clr_stop_det)
			return I2C_STATE_MST_RD_DATA_NACK;

		if(regs->rxflr != 0)
			return I2C_STATE_MST_RD_DATA_ACK;

		return I2C_STATE_NONE;

	case I2C_STATE_SLA_RD_DATA_ACK:
		if(regs->rxflr > 0)
			return I2C_STATE_SLA_RD_DATA_ACK;

		if(regs->clr_stop_det)
			return I2C_STATE_SLA_RD_DATA_NACK;

		return I2C_STATE_NONE;

	case I2C_STATE_SLA_WR_DATA_ACK:
		if(regs->clr_stop_det)
			return I2C_STATE_SLA_WR_DATA_NACK;

		if(regs->raw_intr_stat & (0x1 << INT_TX_EMPTY))
			return I2C_STATE_SLA_WR_DATA_ACK;

		return I2C_STATE_NONE;

	case I2C_STATE_NONE:
		if(regs->clr_rd_req)
			return I2C_STATE_SLA_WR_MATCH;

		if(i2c->mode == I2C_SLAVE && regs->rxflr > 0)
			return I2C_STATE_SLA_RD_MATCH;

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
	if(i2c->state == I2C_STATE_MST_RD_ACK || i2c->state == I2C_STATE_MST_RD_DATA_ACK){
		i2c->state = I2C_STATE_MST_RD_DATA_ACK;

		return tx(0x0, remaining, I2C_READ, remaining <= FIFO_DEPTH, i2c);
	}

	/* slave read */
	i2c->state = (remaining > 1) ? I2C_STATE_SLA_RD_DATA_ACK : I2C_STATE_SLA_RD_DATA_NACK;

	return remaining;
}

static size_t acked(size_t staged, void *hw){
	i2c_regs_t *regs = ((dev_data_t*)hw)->dtd->regs;


	// do not acknowledge bytes that have been flushed due to an abort
	if(regs->clr_tx_abrt)
		return staged - bits_get(regs->tx_abrt_source, ABRT_SRC_TX_FLUSH_CNT, 0x1ff);

	return staged;
}

static void idle(bool addressable, bool stop, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	i2c->mode = I2C_SLAVE;
	i2c->state = I2C_STATE_NONE;

	if(set_mode(i2c, 0, addressable) != 0)
		i2c->state = I2C_STATE_ERROR;
}

static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	i2c->mode = I2C_MASTER;
	i2c->state = (cmd == I2C_READ) ? I2C_STATE_MST_RD_ACK : I2C_STATE_MST_WR_ACK;

	if(set_mode(i2c, slave, false) != 0)
		i2c->state = I2C_STATE_ERROR;
}

static size_t read(uint8_t *buf, size_t n, void *hw){
	return rx(buf, n, hw);
}

static size_t write(uint8_t *buf, size_t n, bool last, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	i2c->state = (i2c->mode == I2C_MASTER) ? I2C_STATE_MST_WR_DATA_ACK : I2C_STATE_SLA_WR_DATA_ACK;

	return tx(buf, n, I2C_WRITE, last, i2c);
}

static int set_mode(dev_data_t *i2c, uint8_t slave, bool addressable){
	i2c_regs_t *regs = i2c->dtd->regs;
	uint32_t con;


	if((regs->status & (0x1 << STATUS_SLV_ACTIVITY)) && (i2c->mode == I2C_MASTER))
		return_errno(E_INUSE);

	con = regs->con & ~((0x1 << CON_IC_SLAVE_DISABLED) | (0x1 << CON_MASTER_MODE));
	con |= ((i2c->mode == I2C_MASTER) << CON_MASTER_MODE);
	con |= ((i2c->mode == I2C_MASTER || !addressable) << CON_IC_SLAVE_DISABLED);

	// do not change the configuration if the hardware is already in the desired state
	if(con == regs->con && slave == bits_get(regs->tar, TAR_IC_TAR, 0x3ff))
		return 0;

	enable(false, i2c);

	regs->con = con;
	regs->tar = (slave << TAR_IC_TAR)
			  | ((slave == 0) << TAR_SPECIAL)
			  | (0x0 << TAR_GC_OR_START)
			  ;

	enable(true, i2c);

	return 0;
}

static void reset(dev_data_t *i2c){
	i2c_regs_t *regs = i2c->dtd->regs;
	i2c_regs_t cfg;


	// store register values
	cfg.con = regs->con;
	cfg.slv_data_nack_only = regs->slv_data_nack_only;
	cfg.ack_gen_call = regs->ack_gen_call;
	cfg.sar = regs->sar;
	cfg.intr_mask = regs->intr_mask;
	cfg.rx_tl = regs->rx_tl;
	cfg.tx_tl = regs->tx_tl;
	cfg.dma_cr = regs->dma_cr;
	cfg.dma_rdlr = regs->dma_rdlr;
	cfg.dma_tdlr = regs->dma_tdlr;

	cfg.fs_spklen = regs->fs_spklen;
	cfg.fs_scl_lcnt = regs->fs_scl_lcnt;
	cfg.fs_scl_hcnt = regs->fs_scl_hcnt;
	cfg.ss_scl_lcnt = regs->ss_scl_lcnt;
	cfg.ss_scl_hcnt = regs->ss_scl_hcnt;
	cfg.sda_hold = regs->sda_hold;
	cfg.sda_setup = regs->sda_setup;

	// reset hardware
	rp2040_resets_halt(0x1 << i2c->dtd->reset_id);
	rp2040_resets_release(0x1 << i2c->dtd->reset_id);

	// restore register values
	regs->con = cfg.con;
	regs->slv_data_nack_only = cfg.slv_data_nack_only;
	regs->ack_gen_call = cfg.ack_gen_call;
	regs->sar = cfg.sar;
	regs->intr_mask = cfg.intr_mask;
	regs->rx_tl = cfg.rx_tl;
	regs->tx_tl = cfg.tx_tl;
	regs->dma_cr = cfg.dma_cr;
	regs->dma_rdlr = cfg.dma_rdlr;
	regs->dma_tdlr = cfg.dma_tdlr;

	regs->fs_spklen = cfg.fs_spklen;
	regs->fs_scl_lcnt = cfg.fs_scl_lcnt;
	regs->fs_scl_hcnt = cfg.fs_scl_hcnt;
	regs->ss_scl_lcnt = cfg.ss_scl_lcnt;
	regs->ss_scl_hcnt = cfg.ss_scl_hcnt;
	regs->sda_hold = cfg.sda_hold;
	regs->sda_setup = cfg.sda_setup;
}

static void enable(bool en, dev_data_t *i2c){
	i2c_regs_t *regs = i2c->dtd->regs;


	// hard reset the i2c hardware if it is not in idle state when trying
	// to disable it, since disabling it doesn't work otherwise
	if(!en && (regs->status & (0x1 << STATUS_SLV_ACTIVITY)))
		reset(i2c);

	regs->enable = (en << ENABLE_ENABLE)
				 | (0x0 << ENABLE_TX_CMD_BLOCK)
				 | (0x0 << ENABLE_ABORT)
				 ;

	while((bool)(regs->enable_status & (0x1 << ENABLE_STATUS_IC_EN)) != en);
}

static size_t tx(uint8_t *buf, size_t n, i2c_cmd_t cmd, bool stop, dev_data_t *i2c){
	i2c_regs_t *regs = i2c->dtd->regs;


	for(size_t i=0; i<n; i++){
		if((regs->status & (0x1 << STATUS_TX_FIFO_NOT_FULL)) == 0)
			return i;

		regs->data_cmd = ((buf ? buf[i] : 0x0) << DATA_CMD_DAT)
					   | ((cmd == I2C_READ) << DATA_CMD_CMD)
					   | ((stop && i + 1 == n) << DATA_CMD_STOP)
					   ;
	}

	return n;
}

static size_t rx(uint8_t *buf, size_t n, dev_data_t *i2c){
	i2c_regs_t *regs = i2c->dtd->regs;


	for(size_t i=0; i<n; i++){
		if(regs->rxflr == 0)
			return i;

		buf[i] = (uint8_t)(regs->data_cmd & 0xff);
	}

	return n;
}
