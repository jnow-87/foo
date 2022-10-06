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
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/ioctl.h>
#include <sys/vram.h>


/* local/static prototypes */
static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data, size_t n);
static void *mmap(devfs_dev_t *dev, fs_filed_t *fd, size_t n);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	devfs_ops_t ops;
	vram_t *vram;


	vram = vram_create(dt_itf, dt_data);

	if(vram == 0x0)
		return 0x0;

	memset(&ops, 0x0, sizeof(ops));

	ops.ioctl = ioctl;
	ops.mmap = mmap;

	if(devfs_dev_register(name, &ops, vram) == 0x0)
		vram_destroy(vram);

	return 0x0;
}

driver_probe("vram,raw", probe);

static int ioctl(devfs_dev_t *dev, fs_filed_t *fd, int request, void *data, size_t n){
	vram_t *vram;


	vram = (vram_t*)dev->data;

	if(n != sizeof(vram_cfg_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:	memcpy(data, &vram->cfg, n); return E_OK;
	case IOCTL_CFGWR:	return vram_configure(vram, data);
	default:			return_errno(E_NOSUP);
	};
}

static void *mmap(devfs_dev_t *dev, fs_filed_t *fd, size_t n){
	vram_t *vram;


	vram = (vram_t*)dev->data;

	if(n != vram->npages * vram->cfg.width + vram_ndirty(vram->npages))
		goto_errno(err, E_INVAL);

	return ummap(vram->ram);


err:
	return 0x0;
}
