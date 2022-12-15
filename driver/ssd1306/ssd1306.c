/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <driver/vram.h>
#include <driver/i2c.h>
#include <sys/compiler.h>
#include <sys/blob.h>
#include <sys/errno.h>
#include <sys/vram.h>


/* macros */
// i2c wrapper
#define I2C_WRITE(i2c, slave, buf)		i2c_write(i2c, slave, buf, sizeof_array(buf))
#define I2C_WRITE_N(i2c, slave, buf)	i2c_write_n(i2c, slave, buf, sizeof_array(buf))
#define I2C_DATA(...)					((uint8_t []){ __VA_ARGS__ })


/* types */
typedef enum{
	CTRL_CMD_1 = 0x80,
	CTRL_CMD_N = 0x00,
	CTRL_DATA_1 = 0xc0,
	CTRL_DATA_N = 0x40,
} ctrl_byte_t;

typedef enum{
	// base commands
	CMD_DSP_ON = 0xaf,				// on
	CMD_DSP_OFF = 0xae,				// off
	CMD_DSP_MODE_RAM = 0xa4,		// pixels = ram
	CMD_DSP_MODE_ALLON = 0xa5,		// pixels = on
	CMD_DSP_MODE_NORM = 0xa6,		// pixels = ram
	CMD_DSP_MODE_INVERSE = 0xa7,	// pixesl = !ram
	CMD_DSP_CONTRAST = 0x81,		// contrast

	// addressing commands
	CMD_ADDR_MODE = 0x20,			// addressing mode
	CMD_ADDR_PAGE_COL_LOW = 0x00,	// low nibble of start column
	CMD_ADDR_PAGE_COL_HIGH = 0x10,	// high nibble of start column
	CMD_ADDR_PAGE_START = 0xb0,		// current page

	// layout commands
	CMD_LAY_START_ROW = 0x40,		// start line
	CMD_LAY_COLS = 0x21,			// start and end columns
	CMD_LAY_ROWS = 0x22,			// start and end rows
	CMD_LAY_VSHIFT = 0xd3,			// vertical offset
	CMD_LAY_REMAP_COLS = 0xa0,		// remap columns left <-> right
	CMD_LAY_REMAP_ROWS = 0xc0,		// remap rows up <-> down

	// hardware configuration commands
	CMD_HW_MUX = 0xa8,				// frequency mux
	CMD_HW_CLOCK = 0xd5,			// clock setting
	CMD_HW_PRECHARGE_PERIOD = 0xd9,	// pre-charge period
	CMD_HW_VCOM = 0xdb,				// v_com voltage
	CMD_HW_CHARGEPUMP = 0x8d,		// charge punp
	CMD_HW_ROWMAP = 0xda,			// pin to row mapping
} cmd_t;

typedef enum{
	AM_HOR = 0x0,
	AM_VERT = 0x1,
	AM_PAGE = 0x2,
} addr_mode_t;

typedef struct{
	uint8_t slave;
	i2c_t *i2c;		/**< set by the driver */
} dt_data_t;


/* local/static prototypes */
static int configure(vram_cfg_t *cfg, void *hw);
static int write_page(uint8_t *buf, size_t page, vram_cfg_t *cfg, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	i2c_t *dti = (i2c_t*)dt_itf;
	vram_itf_t *itf;


	itf = kmalloc(sizeof(vram_itf_t));

	if(itf == 0x0)
		return 0x0;

	dtd->i2c = dti;

	itf->configure = configure;
	itf->write_page = write_page;
	itf->hw = dtd;

	return itf;
}

driver_probe("ssd1306", probe);

static int configure(vram_cfg_t *cfg, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;
	int r;


	r = I2C_WRITE(dtd->i2c, dtd->slave,
		I2C_DATA(
			CTRL_CMD_N,
			CMD_HW_MUX, 0x3f,
			CMD_HW_ROWMAP, 0x12,
			CMD_HW_CLOCK, 0x80,
			CMD_HW_PRECHARGE_PERIOD, 0x22,
			CMD_HW_VCOM, 0x20,
			CMD_ADDR_MODE, AM_PAGE,
			(cfg->flags & VRFL_INVERSE) ? CMD_DSP_MODE_INVERSE : CMD_DSP_MODE_RAM,
			CMD_DSP_MODE_NORM,
			CMD_LAY_VSHIFT, 0x0,
			CMD_LAY_START_ROW | 0x0,
			CMD_LAY_REMAP_COLS | ((cfg->flags & VRFL_MIRROR_HOR) ? 0x1 : 0x0),
			CMD_LAY_REMAP_ROWS | ((cfg->flags & VRFL_MIRROR_VERT) ? 0x8 : 0x0),
			CMD_DSP_CONTRAST, cfg->contrast,
			CMD_LAY_COLS, 0, cfg->width - 1,
			CMD_LAY_ROWS, 0, vram_npages(cfg) - 1,
			CMD_ADDR_PAGE_COL_LOW | 0x0,
			CMD_ADDR_PAGE_COL_HIGH | 0x0,
		)
	);

	// turn display and charge pump on
	r |= I2C_WRITE(dtd->i2c, dtd->slave, I2C_DATA(CTRL_CMD_N, CMD_HW_CHARGEPUMP, 0x14, CMD_DSP_ON));

	return 0;
}

static int write_page(uint8_t *buf, size_t page, vram_cfg_t *cfg, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;
	uint8_t cmd = CTRL_DATA_N;
	int r;


	// set page address
	r = I2C_WRITE(dtd->i2c, dtd->slave, I2C_DATA(CTRL_CMD_1, CMD_ADDR_PAGE_START | page));

	// write data
	r |= I2C_WRITE_N(dtd->i2c, dtd->slave, BLOBS(BLOB(&cmd, 1), BLOB(buf, cfg->width)));

	return r;
}
