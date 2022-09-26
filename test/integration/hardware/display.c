/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/math.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/vram.h>
#include <X11/Xlib.h>
#include <user/debug.h>


/* macros */
#define PIXEL_ISSET(pixels, n)	(((pixels) & (0x1 << (n))) != 0)


/* types */
typedef struct{
	Display *dsp;
	int screen;
	Window win;
	GC gc;

	uint16_t width,
			 height,
			 npages;
	uint8_t scale;

	unsigned long int refresh_ms;

	uint8_t *dirty,
			*ram;
	int shm_id;
} ctx_t;


/* local/static prototypes */
static void draw_page(uint16_t page);
static void draw_pixel(uint16_t x, uint16_t y, uint8_t scale);

static int xerror_hdlr(Display *dsp, XErrorEvent *e);


/* static variables */
static ctx_t ctx = { 0 };
static unsigned char xerror = 0;


/* global functions */
int display_init(void){
	ctx.dsp = XOpenDisplay(0x0);

	if(ctx.dsp == 0x0)
		return -1;

	ctx.screen = XDefaultScreen(ctx.dsp);
	ctx.gc = XDefaultGC(ctx.dsp, ctx.screen);
	ctx.refresh_ms = 1000;

	return 0;
}

void display_cleanup(void){
	if(xerror != 0)
		return;

	if(ctx.win != 0){
		XUnmapWindow(ctx.dsp, ctx.win);
		XDestroyWindow(ctx.dsp, ctx.win);
	}

	if(ctx.dsp != 0x0)
		XCloseDisplay(ctx.dsp);

	shmdt(ctx.ram);
	shmctl(ctx.shm_id, IPC_RMID, 0x0);
}

void display_poll(void){
	uint16_t i;
	struct timespec ts;


	/* sleep for refresh period */
	ts.tv_sec = ctx.refresh_ms / 1000;
	ts.tv_nsec = (ctx.refresh_ms % 1000) * 1000000;

	while(nanosleep(&ts, &ts) && errno == EINTR);

	/* return if window is not initiased */
	if(ctx.win == 0)
		return;

	/* update screen */
	for(i=0; i<ctx.npages; i++){
		if(vram_isdirty(i, ctx.dirty))
			draw_page(i);

		vram_makedirty(i, ctx.dirty, false);
	}

	XFlush(ctx.dsp);
}

int display_configure(int shm_id, uint8_t scale, vram_cfg_t *cfg){
	ctx.refresh_ms = cfg->refresh_ms;

	if(ctx.win != 0)
		return 0;

	ctx.width = cfg->width;
	ctx.height = cfg->height;
	ctx.npages = vram_npages(cfg);
	ctx.scale = scale;

	ctx.shm_id = shm_id;
	ctx.ram = shmat(shm_id, 0x0, 0);
	ctx.dirty = ctx.ram + ctx.npages * cfg->width;

	if(ctx.ram == 0x0)
		return -1;

	XSetErrorHandler(xerror_hdlr);

	ctx.win = XCreateSimpleWindow(
		ctx.dsp,
		XRootWindow(ctx.dsp, ctx.screen),
		0,
		0,
		cfg->width * scale,
		cfg->height * scale,
		0,
		0,
		0
	);

	XStoreName(ctx.dsp, ctx.win, "brickos display");
	XMapWindow(ctx.dsp, ctx.win);
	XClearWindow(ctx.dsp, ctx.win);
	XFlush(ctx.dsp);

	return 0;
}


/* local functions */
static void draw_page(uint16_t page){
	uint16_t i,
			 j;
	uint8_t pixel_state;
	uint8_t pixels;


	for(pixel_state=0; pixel_state<2; pixel_state++){
		if(pixel_state == 0)	XSetForeground(ctx.dsp, ctx.gc, BlackPixel(ctx.dsp, ctx.screen));
		else					XSetForeground(ctx.dsp, ctx.gc, WhitePixel(ctx.dsp, ctx.screen));

		for(i=0; i<ctx.width; i++){
			pixels = ctx.ram[page * ctx.width + i];

			for(j=0; j<8; j++){
				if(PIXEL_ISSET(pixels, j) == pixel_state)
					draw_pixel(i, page * 8 + j, ctx.scale);
			}
		}
	}
}

static void draw_pixel(uint16_t x, uint16_t y, uint8_t scale){
	uint16_t i,
			 j;


	x *= scale;
	y *= scale;

	for(i=0; i<scale; i++){
		for(j=0; j<scale; j++)
			XDrawPoint(ctx.dsp, ctx.win, ctx.gc, x + i, y + j);
	}
}

static int xerror_hdlr(Display *dsp, XErrorEvent *e){
	char s[32];


	xerror = e->error_code;
	XGetErrorText(dsp, e->error_code, s, 32);
	EEXIT("x11 error: %s\n", s);

	return 0;
}
