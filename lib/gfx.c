/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <lib/stdlib.h>
#include <lib/unistd.h>
#include <lib/gfx.h>
#include <sys/math.h>
#include <sys/ioctl.h>
#include <sys/font.h>
#include <sys/vram.h>


/* types */
typedef struct{
	gfx_uint_t num,
			   denom,
			   v;
	int8_t sign;
} delta_t;


/* local/static prototypes */
static int8_t delta_inc(delta_t *d);
static void delta_init(delta_t *d, gfx_int_t num, gfx_uint_t denom);


/* global functions */
gfx_ctx_t *gfx_alloc(char const *dev){
	uint16_t npages;
	gfx_ctx_t *gc;


	gc = malloc(sizeof(gfx_ctx_t));

	if(gc == 0x0)
		goto err_0;

	gc->fd = open(dev, O_RDWR);

	if(gc->fd < 0)
		goto err_1;

	if(ioctl(gc->fd, IOCTL_CFGRD, &gc->cfg) != 0)
		goto err_2;

	npages = vram_npages(&gc->cfg);
	gc->mem = mmap(gc->fd, npages * gc->cfg.width + vram_ndirty(npages));
	gc->dirty = gc->mem + npages * gc->cfg.width;

	if(gc->mem == 0x0)
		goto err_2;

	gfx_set_font(gc, 0x0);

	return gc;


err_2:
	close(gc->fd);

err_1:
	free(gc);

err_0:
	return 0x0;
}

void gfx_free(gfx_ctx_t *gc){
	munmap(gc->mem);
	close(gc->fd);

	free(gc);
}

void gfx_set_font(gfx_ctx_t *gc, char const *name){
	gc->font = font_resolve(name);
}

void gfx_clearscreen(gfx_ctx_t *gc){
	uint16_t npages;


	npages = vram_npages(&gc->cfg);

	memset(gc->mem, 0x0, npages * gc->cfg.width);
	memset(gc->dirty, 0xff, vram_ndirty(npages));
}

void gfx_draw_point(gfx_ctx_t *gc, gfx_uint_t x, gfx_uint_t y){
	uint16_t page;


	page = y / 8;

	gc->mem[page * gc->cfg.width + x] |= (0x1 << (y & 0x7));
	vram_makedirty(page, gc->dirty, true);
}

void gfx_draw_points(gfx_ctx_t *gc, gfx_vec2d_t *pts, size_t n){
	size_t i;


	for(i=0; i<n; i++)
		gfx_draw_point(gc, pts[i].x, pts[i].y);
}

void gfx_draw_line(gfx_ctx_t *gc, gfx_uint_t x0, gfx_uint_t y0, gfx_uint_t x1, gfx_uint_t y1){
	gfx_int_t i,
			  x,
			  y,
			  max;
	delta_t dx,
			dy;


	x = x1 - x0;
	y = y1 - y0;
	max = MAX(ABS(x), ABS(y));

	delta_init(&dx, x, max);
	delta_init(&dy, y, max);

	for(i=0; i<=max; i++){
		gfx_draw_point(gc, x0, y0);

		x0 += delta_inc(&dx);
		y0 += delta_inc(&dy);
	}
}

void gfx_draw_polygone(gfx_ctx_t *gc, gfx_vec2d_t *pts, size_t n){
	size_t i,
		   j;


	for(i=0, j=1; i<n; i++, j++){
		if(j >= n)
			j = 0;

		gfx_draw_line(gc, pts[i].x, pts[i].y, pts[j].x, pts[j].y);
	}
}

void gfx_draw_rectangle(gfx_ctx_t *gc, gfx_uint_t x, gfx_uint_t y, gfx_uint_t width, gfx_uint_t height){
	gfx_draw_polygone(
		gc,
		(gfx_vec2d_t []){
			{ x, y },
			{ x + width, y },
			{ x + width, y + height },
			{ x, y + height },
		},
		4
	);
}

void gfx_draw_string(gfx_ctx_t *gc, gfx_uint_t x, gfx_uint_t y, char const *s){
	size_t i;
	uint16_t page,
			 column;


	page = y / 8;

	for(i=0; s[i]!=0; i++){
		column = x + i * gc->font->width;

		if(column >= gc->cfg.width)
			break;

		memcpy(
			gc->mem + page * gc->cfg.width + column,
			font_char(s[i], gc->font),
			MIN(gc->font->width, gc->cfg.width - column)
		);
	}

	vram_makedirty(page, gc->dirty, true);
}


/* local functions */
static void delta_init(delta_t *d, gfx_int_t num, gfx_uint_t denom){
	d->v = 0;
	d->sign = (num < 0) ? -1 : 1;
	d->num = num * d->sign;
	d->denom = denom;
}

static int8_t delta_inc(delta_t *d){
	gfx_uint_t v0,
			   v1,
			   d_2;


	v0 = d->v;
	v1 = d->v + d->num;
	d_2 = d->denom >> 1;

	d->v = (v1 >= d->denom) ? 0 : v1;

	if(v1 >= d_2 && v0 < d_2)
		return d->sign;

	return 0;
}
