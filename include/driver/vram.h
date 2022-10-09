/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_VRAM_H
#define DRIVER_VRAM_H


#include <kernel/timer.h>
#include <sys/types.h>
#include <sys/vram.h>


/* types */
typedef struct{
	int (*configure)(vram_cfg_t *cfg, void *hw);
	int (*write_page)(uint8_t *buf, size_t page, vram_cfg_t *cfg, void *hw);

	void *hw;
} vram_itf_t;

typedef struct{
	vram_cfg_t cfg;
	vram_itf_t itf;

	uint8_t *dirty;
	uint8_t *ram;

	size_t npages,
		   page_offset;

	ktimer_t timer;
} vram_t;


/* prototypes */
vram_t *vram_create(vram_itf_t *itf, vram_cfg_t *cfg);
void vram_destroy(vram_t *vram);

int vram_configure(vram_t *vram, vram_cfg_t *cfg);

int vram_write_page(vram_t *vram, size_t page, size_t column, size_t n, uint8_t pattern);
int vram_write_pages(vram_t *vram, size_t start, size_t npages, uint8_t pattern);
int vram_write_pattern(vram_t *vram, uint8_t pattern);
int vram_write_block(vram_t *vram, size_t page, size_t column, uint8_t *block, size_t bsize, size_t n);

int vram_invert_page(vram_t *vram, size_t page, size_t column, size_t n);
int vram_invert_pages(vram_t *vram, size_t start, size_t npages);

int vram_scroll(vram_t *vram, ssize_t npages);


#endif // DRIVER_VRAM_H
