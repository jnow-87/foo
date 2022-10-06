/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/math.h>
#include <sys/vram.h>


/* global functions */
uint16_t vram_npages(vram_cfg_t *cfg){
	return MAX(1, cfg->height / 8);
}

uint16_t vram_ndirty(uint16_t npages){
	return MAX(1, npages / 8);
}

bool vram_isdirty(uint16_t page, uint8_t *dirty){
	return ((dirty[page >> 3] & (0x1 << (page & 0x7))) != 0);
}

void vram_makedirty(uint16_t page, uint8_t *dirty, bool set){
	dirty[page >> 3] &= ~(0x1 << (page & 0x7));
	dirty[page >> 3] |= ((set ? 0x1 : 0x0) << (page & 0x7));
}
