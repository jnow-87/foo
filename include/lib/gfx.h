/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_GFX
#define LIB_GFX


#include <config/config.h>
#include <sys/types.h>
#include <sys/font.h>
#include <sys/vram.h>


/* types */
typedef UINT(CONFIG_GFX_INT_WIDTH) gfx_uint_t;
typedef INT(CONFIG_GFX_INT_WIDTH) gfx_int_t;

typedef struct{
	gfx_uint_t x,
			   y;
} gfx_vec2d_t;

typedef struct{
	int fd;
	vram_cfg_t cfg;

	font_t *font;

	uint8_t *mem,
			*dirty;
} gfx_ctx_t;


/* prototypes */
gfx_ctx_t *gfx_alloc(char const *dev);
void gfx_free(gfx_ctx_t *gc);

void gfx_set_font(gfx_ctx_t *gc, char const *name);

void gfx_clearscreen(gfx_ctx_t *gc);

void gfx_draw_point(gfx_ctx_t *gc, gfx_uint_t x, gfx_uint_t y);
void gfx_draw_points(gfx_ctx_t *gc, gfx_vec2d_t *pts, size_t n);
void gfx_draw_line(gfx_ctx_t *gc, gfx_uint_t x0, gfx_uint_t y0, gfx_uint_t x1, gfx_uint_t y1);
void gfx_draw_polygone(gfx_ctx_t *gc, gfx_vec2d_t *pts, size_t n);
void gfx_draw_rectangle(gfx_ctx_t *gc, gfx_uint_t x, gfx_uint_t y, gfx_uint_t width, gfx_uint_t height);
void gfx_draw_string(gfx_ctx_t *gc, gfx_uint_t x, gfx_uint_t y, char const *s);


#endif // LIB_GFX
