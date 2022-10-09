/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/hardware.h>
#include <arch/x86/linux.h>
#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/vram.h>
#include <sys/errno.h>
#include <sys/math.h>
#include <sys/string.h>
#include <sys/vram.h>


/* types */
typedef struct{
	uint8_t scale;
} __packed dt_data_t;

typedef struct{
	uint8_t *dirty,
			*ram,
			*shm;

	int shm_id;

	dt_data_t *dsp_cfg;
} dev_data_t;


/* local/static prototypes */
static int configure(vram_cfg_t *cfg, void *hw);
static int write_page(uint8_t *buf, size_t page, vram_cfg_t *cfg, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd;
	vram_itf_t *itf;
	dev_data_t *dsp;


	dtd = (dt_data_t*)dt_data;

	dsp = kcalloc(1, sizeof(dev_data_t));
	itf = kmalloc(sizeof(vram_itf_t));

	if(dsp == 0x0 || itf == 0x0)
		goto err;

	dsp->dsp_cfg = dtd;

	itf->configure = configure;
	itf->write_page = write_page;
	itf->hw = dsp;

	return itf;


err:
	kfree(dsp);
	kfree(itf);

	return 0x0;
}

driver_probe("x11", probe);

static int configure(vram_cfg_t *cfg, void *hw){
	uint16_t npages;
	dev_data_t *dsp;
	x86_hw_op_t op;


	dsp = (dev_data_t*)hw;
	npages = vram_npages(cfg);

	if(dsp->shm != 0x0)
		return 0;

	/* allocate shared memory */
	dsp->shm_id = lnx_shmget(npages * cfg->width + vram_ndirty(npages));

	if(dsp->shm_id < 0)
		goto_errno(err_0, E_NOMEM);

	dsp->shm = lnx_shmat(dsp->shm_id);

	if(dsp->shm == (void*)-1)
		goto_errno(err_1, E_NOMEM);

	dsp->ram = dsp->shm;
	dsp->dirty = dsp->shm + npages * cfg->width;

	/* configure hardware */
	op.num = HWO_DISPLAY_CFG;
	op.display.cfg = *cfg;
	op.display.shm_id = dsp->shm_id;
	op.display.scale = dsp->dsp_cfg->scale;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);

	return 0;


err_1:
	lnx_shmrm(dsp->shm_id, dsp->shm);

err_0:
	return -errno;
}

static int write_page(uint8_t *buf, size_t page, vram_cfg_t *cfg, void *hw){
	dev_data_t *dsp;


	dsp = (dev_data_t*)hw;

	memcpy(dsp->ram + page * cfg->width, buf, cfg->width);
	vram_makedirty(page, dsp->dirty, true);

	return 0;
}
