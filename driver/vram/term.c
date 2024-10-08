/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/memory.h>
#include <driver/term.h>
#include <driver/vram.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/font.h>
#include <sys/vram.h>


/* types */
typedef struct{
	vram_t *vram;
	term_itf_t itf;

	font_t *font;

	uint16_t line,
			 column;
} dev_data_t;


/* local/static prototypes */
static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw);
static size_t puts(char const *s, size_t n, bool blocking, void *hw);
static size_t gets(char *s, size_t n, void *hw);
static int cursor(uint16_t line, uint16_t column, bool toggle, void *hw);
static int scroll(int16_t lines, void *hw);
static int erase(term_erase_t type, uint16_t n, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *term;


	term = kmalloc(sizeof(dev_data_t));

	if(term == 0x0)
		return 0x0;

	term->vram = dt_itf;
	term->font = font_resolve(0x0);
	term->line = 0;
	term->column = 0;

	term->itf.configure = configure;
	term->itf.puts = puts;
	term->itf.gets = gets;
	term->itf.cursor = cursor;
	term->itf.scroll = scroll;
	term->itf.erase = erase;
	term->itf.error = 0x0;

	term->itf.hw = term;
	term->itf.cfg = term->vram->cfg;
	term->itf.cfg_size = sizeof(vram_cfg_t);
	term->itf.rx_int = 0;
	term->itf.tx_int = 0;

	return &term->itf;
}

driver_probe("vram,term", probe);

static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw){
	dev_data_t *term = (dev_data_t*)hw;
	vram_cfg_t *cfg = (vram_cfg_t*)hw_cfg;


	term_cfg->lines = cfg->height / term->font->height;
	term_cfg->columns = cfg->width / term->font->width;

	return vram_configure(term->vram, cfg);
}

static size_t puts(char const *s, size_t n, bool blocking, void *hw){
	dev_data_t *term = (dev_data_t*)hw;


	for(size_t i=0; i<n; i++){
		if(vram_write_block(term->vram, term->line, term->column, font_char(s[i], term->font), term->font->width, 1) != 0)
			return n;

		term->column += term->font->width;
	}

	return n;
}

static size_t gets(char *s, size_t n, void *hw){
	set_errno(E_NOSUP);

	return 0;
}

static int cursor(uint16_t line, uint16_t column, bool toggle, void *hw){
	dev_data_t *term = (dev_data_t*)hw;


	term->line = line * term->font->height / 8;
	term->column = column * term->font->width;

	if(!toggle)
		return 0;

	return vram_invert_page(term->vram, term->line, term->column, term->font->width);
}

static int scroll(int16_t lines, void *hw){
	dev_data_t *term = (dev_data_t*)hw;


	return vram_scroll(term->vram, lines * term->font->height / 8);
}

static int erase(term_erase_t type, uint16_t n, void *hw){
	dev_data_t *term = (dev_data_t*)hw;
	int r = 0;
	uint16_t x;


	/* clear multiple lines */
	if(type & TE_MULTILINE){
		if(type & TE_TO_START){
			x = (n == 0) ? term->line : n * term->font->height / 8;
			r |= vram_write_pages(term->vram, term->line - x, x, 0x0);
		}

		if(type & TE_TO_END){
			x = (n == 0) ? vram_npages(term->vram->cfg) - (term->line + 1) : n * term->font->height / 8;

			if(term->line + 1 < term->vram->cfg->height)
				r |= vram_write_pages(term->vram, term->line + 1, x, 0x0);
		}
	}

	/* clear on current line */
	if(type & TE_TO_START){
		x = (n == 0) ? term->column : n * term->font->width;
		r |= vram_write_page(term->vram, term->line, term->column - x, x, 0x0);
	}

	if(type & TE_TO_END){
		x = (n == 0) ? term->vram->cfg->width - term->column : n * term->font->width;
		r |= vram_write_page(term->vram, term->line, term->column, x, 0x0);
	}

	return r;
}
