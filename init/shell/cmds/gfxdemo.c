/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <unistd.h>
#include <gfx.h>
#include <shell/cmd.h>
#include <shell/shell.h>
#include <sys/string.h>


/* macros */
#define DEFAULT_DEV		"/dev/vram0-raw"


/* local functions */
static int exec(int argc, char **argv){
	gfx_ctx_t *gc;


	gc = gfx_alloc((argc > 1) ? argv[1] : DEFAULT_DEV);

	if(gc == 0x0)
		return -ERROR("creating gfx context");

	gfx_draw_point(gc, gc->cfg.width / 2, 1);
	gfx_draw_points(
		gc,
		(gfx_vec2d_t []){
			{ 0, 0 },
			{ gc->cfg.width - 1, 0},
			{ 0, gc->cfg.height - 1 },
			{ gc->cfg.width - 1, gc->cfg.height - 1},
		},
		4
	);

	gfx_draw_line(gc, 20, 20, 30, 20);
	gfx_draw_line(gc, 20, 20, 30, 25);
	gfx_draw_line(gc, 20, 20, 30, 30);
	gfx_draw_line(gc, 20, 20, 25, 30);

	gfx_draw_line(gc, 20, 20, 20, 30);
	gfx_draw_line(gc, 20, 20, 15, 30);
	gfx_draw_line(gc, 20, 20, 10, 30);
	gfx_draw_line(gc, 20, 20, 10, 25);

	gfx_draw_line(gc, 20, 20, 10, 20);
	gfx_draw_line(gc, 20, 20, 10, 15);
	gfx_draw_line(gc, 20, 20, 10, 10);
	gfx_draw_line(gc, 20, 20, 15, 10);

	gfx_draw_line(gc, 20, 20, 20, 10);
	gfx_draw_line(gc, 20, 20, 25, 10);
	gfx_draw_line(gc, 20, 20, 30, 10);
	gfx_draw_line(gc, 20, 20, 30, 15);

	gfx_draw_line(gc, 40, 10, 41, 40);

	gfx_draw_rectangle(gc, 1, 40, 20, 9);

	gfx_draw_rectangle(gc, 59, 19, 11, 12);
	gfx_draw_polygone(
		gc,
		(gfx_vec2d_t []){
			{ 62, 20 },
			{ 67, 20 },
			{ 69, 25 },
			{ 67, 30 },
			{ 62, 30 },
			{ 60, 25 },
		},
		6
	);

	gfx_draw_rectangle(gc, 79, 19, 11, 22);
	gfx_draw_polygone(
		gc,
		(gfx_vec2d_t []){
			{ 82, 20 },
			{ 87, 20 },
			{ 89, 30 },
			{ 87, 40 },
			{ 82, 40 },
			{ 80, 30 },
		},
		6
	);

	gfx_draw_string(gc, 112, 10, "foo");

	return 0;
}

command("gfxdemo", exec);
