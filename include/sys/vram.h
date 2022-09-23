/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_VRAM_H
#define SYS_VRAM_H


#include <sys/types.h>
#include <sys/compiler.h>


/* types */
typedef enum{
	VRFL_ORIEN_HOR = 0x0,
	VRFL_ORIEN_VERT = 0x1,
	VRFL_MIRROR_HOR = 0x2,
	VRFL_MIRROR_VERT = 0x4,
	VRFL_INVERSE = 0x8,
} vram_flags_t;

typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint16_t height,
			 width;
	uint8_t contrast;

	uint8_t flags;		/**< cf. vram_flags_t */

	uint16_t refresh_ms;
} __packed vram_cfg_t;


/* prototypes */
uint16_t vram_npages(vram_cfg_t *cfg);
uint16_t vram_ndirty(uint16_t npages);

bool vram_isdirty(uint16_t page, uint8_t *dirty);
void vram_makedirty(uint16_t page, uint8_t *dirty, bool set);


#endif // SYS_VRAM_H
