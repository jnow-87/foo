/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/memory.h>
#include <driver/vram.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/ioctl.h>
#include <sys/vram.h>


/* types */
typedef struct{
	vram_cfg_t *cfg;
	uint8_t *ram;
} dev_data_t;


/* local/static prototypes */
static int configure(vram_cfg_t *cfg, void *hw);
static int write_page(uint8_t *data, size_t page, vram_cfg_t *cfg, void *hw);

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data, size_t n);
static void *mmap(devfs_dev_t *dev, fs_filed_t *fd, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *vram;
	vram_itf_t *itf;
	devfs_ops_t ops;


	/* create the vram */
	vram = kcalloc(1, sizeof(dev_data_t));
	itf = kmalloc(sizeof(vram_itf_t));

	if(vram == 0x0 || itf == 0x0)
		goto err;

	itf->configure = configure;
	itf->write_page = write_page;
	itf->hw = vram;

	/* register device */
	memset(&ops, 0x0, sizeof(ops));

	ops.ioctl = ioctl;
	ops.mmap = mmap;

	if(devfs_dev_register(name, &ops, vram) == 0x0)
		goto err;

	return itf;


err:
	kfree(itf);
	kfree(vram);

	return 0x0;
}

driver_probe("loop,vram", probe);

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data, size_t n){
	dev_data_t *vram;


	vram = (dev_data_t*)dev->data;

	if(n != sizeof(vram_cfg_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:	memcpy(data, vram->cfg, n); break;
	case IOCTL_CFGWR:	memcpy(vram->cfg, data, n); break;
	default:			return_errno(E_NOSUP);
	};

	return 0;
}

static void *mmap(devfs_dev_t *dev, fs_filed_t *fd, size_t n){
	dev_data_t *vram;


	vram = (dev_data_t*)dev->data;

	if(n > vram_npages(vram->cfg) * vram->cfg->width)
		goto_errno(err, E_LIMIT);

	return ummap(vram->ram);


err:
	return 0x0;
}

static int configure(vram_cfg_t *cfg, void *hw){
	dev_data_t *vram;


	vram = (dev_data_t*)hw;

	if(vram->ram == 0x0){
		vram->cfg = cfg;
		vram->ram = kcalloc(1, vram_npages(cfg) * cfg->width);
	}
	else
		memcpy(vram->cfg, cfg, sizeof(vram_cfg_t));

	return -errno;
}

static int write_page(uint8_t *data, size_t page, vram_cfg_t *cfg, void *hw){
	size_t i;
	dev_data_t *vram;


	vram = (dev_data_t*)hw;

	if(vram->cfg->flags & VRFL_INVERSE){
		for(i=0; i<cfg->width; i++)
			vram->ram[page * vram->cfg->width + i] = data[i] ^ 0xff;
	}
	else
		memcpy(vram->ram + (page * vram->cfg->width), data, cfg->width);

	return 0;
}
