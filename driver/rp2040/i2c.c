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
typedef enum{
	CMD_WRITE = 0,
	CMD_READ
} cmd_t;

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


#include <kernel/kprintf.h>


/* local/static prototypes */
static int configure(dt_data_t *dtd);
static int read(i2c_t *i2c, uint8_t slave, void *buf, size_t n);
static int write(i2c_t *i2c, uint8_t slave, blob_t *bufs, size_t n);
static void enable(bool en, i2c_regs_t *regs);
int configure_transfer(uint8_t slave, bool read, i2c_regs_t *regs);
static size_t rx(uint8_t *buf, size_t n, bool blocking, i2c_regs_t *regs);
static size_t tx(uint8_t *buf, size_t n, cmd_t cmd, bool blocking, bool stop, i2c_regs_t *regs);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	i2c_ops_t ops;


	ops.read = read;
	ops.write = write;

	if(configure(dtd) != 0)
		return 0x0;

	return i2c_create(&ops, &dtd->cfg, dtd);
}

driver_probe("rp2040,i2c", probe);

static int configure(dt_data_t *dtd){
	uint8_t speeds[] = { 1, 2, 2, 3, 1, 1 };
	i2c_regs_t *regs = dtd->regs;
	i2c_cfg_t *cfg = &dtd->cfg;
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

	regs->con = ((cfg->mode == I2C_MODE_MASTER) << CON_IC_SLAVE_DISABLED)
			  | ((cfg->mode == I2C_MODE_MASTER) << CON_MASTER_MODE)
			  | (speeds[speed] << CON_SPEED)
			  // NOTE so far only 7-bit addresses are supported
			  | (0x0 << CON_IC_10BITADDR_MASTER)
			  | (0x0 << CON_IC_10BITADDR_SLAVE)
			  | (0x1 << CON_RX_FIFO_FULL_HLD_CTRL)
			  | (0x1 << CON_TX_EMPTY_CTRL)
			  |	(0x1 << CON_STOP_DET_IF_ADDRESSED)	// TODO check
			  | ((speed >= I2C_SPD_HIGH) << CON_IC_RESTART_EN)
			  ;
	regs->slv_data_nack_only = (0x0 << SLV_DATA_NACK_ONLY_NACK);
	regs->ack_gen_call = (0x1 << ACK_GENERAL_CALL_ACK_GEN_CALL);

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

	DEBUG("i2c config: mode=%s, addr=%u\n", (cfg->mode == I2C_MODE_MASTER) ? "master" : "slave", cfg->addr);
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

static int read(i2c_t *i2c, uint8_t slave, void *buf, size_t n){
	size_t x = 0;
	dt_data_t *dtd = i2c->hw;
	i2c_regs_t *regs = dtd->regs;


	if(dtd->cfg.mode == I2C_MODE_MASTER){
		if(configure_transfer(slave, true, dtd->regs) != 0)
			return -errno;
	}

	for(size_t i=0; i<n; i+=x){
		x = n - i;

		if(dtd->cfg.mode == I2C_MODE_MASTER){
			x = tx(buf, x, CMD_READ, false, true, regs);
		}

		x = rx(buf + i, x, true, regs);
	}

	return 0;
}

static int write(i2c_t *i2c, uint8_t slave, blob_t *bufs, size_t n){
	dt_data_t *dtd = i2c->hw;
	i2c_regs_t *regs = dtd->regs;


	if(dtd->cfg.mode == I2C_MODE_MASTER){
		if(configure_transfer(slave, false, dtd->regs) != 0)
			return -errno;
	}
	else{
		while((regs->raw_intr_stat & (0x1 << INT_RD_REQ)) == 0);
		DEBUG("got addressed\n");
	}

	for(size_t i=0; i<n; i++)
		(void)tx(bufs[i].buf, bufs[i].len, CMD_WRITE, true, i + 1 == n, regs);

	return 0;
}

static size_t tx(uint8_t *buf, size_t n, cmd_t cmd, bool blocking, bool stop, i2c_regs_t *regs){
	for(size_t i=0; i<n; i++){
		while((regs->status & (0x1 << STATUS_TX_FIFO_NOT_FULL)) == 0){
			if(!blocking)
				return i;
		}

		regs->data_cmd = (buf[i] << DATA_CMD_DAT)
					   | (cmd << DATA_CMD_CMD)
					   | ((stop && i + 1 == n) << DATA_CMD_STOP)
					   ;
	}

	if(blocking)
		while((regs->status & (0x1 << STATUS_TX_FIFO_EMPTY)) == 0);

	return n;
}

static size_t rx(uint8_t *buf, size_t n, bool blocking, i2c_regs_t *regs){
	for(size_t i=0; i<n; i++){
		while((regs->status & (0x1 << STATUS_RX_FIFO_NOT_EMPTY)) == 0){
			if(!blocking)
				return i;
		}

		buf[i] = (uint8_t)(regs->data_cmd & 0xff);
	}

	return n;
}

static void enable(bool en, i2c_regs_t *regs){
	if((regs->status & (0x1 << STATUS_TX_FIFO_EMPTY)) == 0 || (regs->status & (0x1 << STATUS_RX_FIFO_NOT_EMPTY)))
		WARN("abort incomplete operation (status=%#x)\n", regs->status);

	regs->enable = (en << ENABLE_ENABLE)
				 | (0x0 << ENABLE_TX_CMD_BLOCK)
				 | (0x1 << ENABLE_ABORT)
				 ;

	while((regs->enable_status & (0x1 << ENABLE_STATUS_IC_EN)) != en);
}

int configure_transfer(uint8_t slave, bool read, i2c_regs_t *regs){
	bool gen_call = (slave == 0);


	if(gen_call && read)
		return_errno(E_INVAL);

	enable(false, regs);

	// TODO test general call
	regs->tar = (slave << TAR_IC_TAR)
			  | (gen_call << TAR_SPECIAL)
			  | (0x0 << TAR_GC_OR_START)
			  ;

	enable(true, regs);

	return 0;
}
