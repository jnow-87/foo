/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <kernel/timer.h>
#include <driver/vram.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/math.h>
#include <sys/vram.h>


/* macros */
#define RAMIDX(vram, page)	(((page) + (vram)->page_offset) % (vram)->npages)


/* local/static prototypes */
static void refresh_hdlr(void *payload);


/* global functions */
vram_t *vram_create(vram_itf_t *itf, vram_cfg_t *cfg){
	vram_t *vram;


	if(itf->configure(cfg, itf->hw) != 0)
		return 0x0;

	vram = kmalloc(sizeof(vram_t));

	if(vram == 0x0)
		goto err_0;

	vram->npages = vram_npages(cfg);
	vram->ram = kcalloc(1, vram->npages * cfg->width + vram_ndirty(vram->npages));
	vram->dirty = vram->ram + vram->npages * cfg->width;

	if(vram->ram == 0x0)
		goto err_1;

	memset(vram->dirty, 0xff, vram_ndirty(vram->npages));

	vram->page_offset = 0;
	vram->itf = *itf;
	vram->cfg = *cfg;

	ktimer_register(&vram->timer, (uint32_t)cfg->refresh_ms * 1000, refresh_hdlr, vram, true);

	return vram;


err_1:
	vram_destroy(vram);

err_0:
	return 0x0;
}

void vram_destroy(vram_t *vram){
	kfree(vram->ram);
	kfree(vram);
}

int vram_configure(vram_t *vram, vram_cfg_t *cfg){
	if(vram->itf.configure(cfg, vram->itf.hw) != 0)
		return -errno;

	vram->cfg = *cfg;

	return 0;
}

int vram_write_page(vram_t *vram, size_t page, size_t column, size_t n, uint8_t pattern){
	if(page >= vram->npages || column + n > vram->cfg.width)
		return_errno(E_INVAL);

	page = RAMIDX(vram, page);

	memset(vram->ram + (page * vram->cfg.width) + column, pattern, n);
	vram_makedirty(page, vram->dirty, true);

	return 0;
}

int vram_write_pages(vram_t *vram, size_t start, size_t npages, uint8_t pattern){
	size_t i,
		   chunk;


	if(start + npages > vram->npages)
		return_errno(E_INVAL);

	start = RAMIDX(vram, start);

	chunk = MIN(npages, vram->npages - start);
	memset(vram->ram + (start * vram->cfg.width), pattern, chunk * vram->cfg.width);
	memset(vram->ram, pattern, (npages - chunk) * vram->cfg.width);

	for(i=start; npages!=0; npages--, i++){
		if(i == vram->npages)
			i = 0;

		vram_makedirty(i, vram->dirty, true);
	}

	return 0;
}

int vram_write_pattern(vram_t *vram, uint8_t pattern){
	return vram_write_pages(vram, 0, vram->npages, pattern);
}

int vram_write_block(vram_t *vram, size_t page, size_t column, uint8_t *block, size_t bsize, size_t n){
	size_t i;


	if(page >= vram->npages || column + n * bsize > vram->cfg.width)
		return_errno(E_INVAL);

	page = RAMIDX(vram, page);

	for(i=0; i<n; i++)
		memcpy(vram->ram + (page * vram->cfg.width) + column + i * bsize, block, bsize);

	vram_makedirty(page, vram->dirty, true);

	return 0;
}

int vram_invert_page(vram_t *vram, size_t page, size_t column, size_t n){
	size_t i;


	if(page >= vram->npages)
		return_errno(E_INVAL);

	page = RAMIDX(vram, page);

	for(i=0; i<n; i++)
		vram->ram[page * vram->cfg.width + column + i] ^= 0xff;

	vram_makedirty(page, vram->dirty, true);

	return 0;
}

int vram_invert_pages(vram_t *vram, size_t start, size_t npages){
	size_t i;


	for(i=0; i<npages; i++){
		if(vram_invert_page(vram, start + i, 0, vram->cfg.width) != 0)
			return -errno;
	}

	return 0;
}

int vram_scroll(vram_t *vram, ssize_t npages){
	vram->page_offset = (vram->page_offset + npages) % vram->npages;
	memset(vram->dirty, 0xff, vram_ndirty(vram->npages));

	if(npages < 0)
		return vram_write_pages(vram, 0, MIN((size_t)-npages, vram->npages), 0x0);

	npages = MIN((size_t)npages, vram->npages);

	return vram_write_pages(vram, vram->npages - npages, npages, 0x0);
}


/* local functions */
static void refresh_hdlr(void *payload){
	size_t i,
		   page;
	vram_t *vram;


	vram = (vram_t*)payload;
	page = vram->page_offset;

	for(i=0; i<vram->npages; i++, page++){
		if(page >= vram->npages)
			page = 0;

		if(!vram_isdirty(page, vram->dirty))
			continue;

		if(vram->itf.write_page(vram->ram + (page * vram->cfg.width), i, &vram->cfg, vram->itf.hw) != 0)
			WARN("vram refresh failed %s\n", strerror(errno));

		vram_makedirty(page, vram->dirty, false);
	}
}
