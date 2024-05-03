/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/math.h>
#include <sys/ioctl.h>
#include <sys/term.h>
#include <sys/vram.h>
#include <sys/stream.h>
#include <sys/font.h>
#include <test/test.h>


/* macros */
#define LOOP_DEV		"/dev/vram-loop0"
#define TERM_DEV		"/dev/tty-vram0"

#define LINES_MIN		4
#define COLUMNS_MIN		8


/* types */
typedef enum{
	ERASE_LINE = 0x1,
	ERASE_SCREEN = 0x2,
	ERASE_TO_START = 0x4,
	ERASE_TO_END = 0x8,
} erase_t;

typedef struct{
	uint8_t *ram;
	int16_t size,
			width,
			height,
			lines,
			columns;
} vram_t;

typedef struct{
	int fd_loop,
		fd_term;
	vram_t vram,
		   ref;

	uint16_t refresh_ms;
	font_t *font;
} test_data_t;


/* local/static prototypes */
static int test_write(test_data_t *td, char *s, uint16_t line, uint16_t column, char *ref, bool move_cursor);
static int test_erase(test_data_t *td, char *s, uint16_t line, uint16_t column, erase_t type);
static int test_scroll(test_data_t *td, char *s, int16_t lines);

static int prepare(test_data_t *td);
static void cleanup(test_data_t *td);

static int clearscreen(test_data_t *td);
static int setscreen(test_data_t *td);
static int writestr(test_data_t *td, int16_t line, int16_t column, char *s);
static int cmp(test_data_t *td);

static int move(test_data_t *td, uint16_t line, uint16_t column);


/* local functions */
TEST(vramtty_config){
	int r = 0;
	test_data_t td;
	term_vram_cfg_t cfg;


	ASSERT_INT_EQ(prepare(&td), 0);

	// verify scroll and wrap flags are off
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGRD, &cfg), 0);
	r |= TEST_INT_EQ(cfg.term.lflags & (TLFL_SCROLL | TLFL_WRAP), 0);

	// verify scroll enabling works
	r |= TEST_INT_EQ(write(td.fd_term, "\033[r", 3), 3);
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGRD, &cfg), 0);
	r |= TEST_INT_EQ(cfg.term.lflags & (TLFL_SCROLL), TLFL_SCROLL);

	// disable scroll
	cfg.term.lflags &= ~TLFL_SCROLL;
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGWR, &cfg), 0);
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGRD, &cfg), 0);
	r |= TEST_INT_EQ(cfg.term.lflags & (TLFL_SCROLL | TLFL_WRAP), 0);

	// verify wrap enabling works
	r |= TEST_INT_EQ(write(td.fd_term, "\033[7h", 4), 4);
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGRD, &cfg), 0);
	r |= TEST_INT_EQ(cfg.term.lflags & (TLFL_WRAP), TLFL_WRAP);

	// verify wrap disabling works
	r |= TEST_INT_EQ(write(td.fd_term, "\033[7l", 4), 4);
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGRD, &cfg), 0);
	r |= TEST_INT_EQ(cfg.term.lflags & (TLFL_WRAP), 0);

	cleanup(&td);

	return -r;
}

TEST(vramtty_write){
	int r = 0;
	test_data_t td;


	ASSERT_INT_EQ(prepare(&td), 0);

	r |= test_write(&td, "foo", 0, 0, "foo", false);

	cleanup(&td);

	return -r;
}

TEST(vramtty_wrap){
	int r = 0;
	test_data_t td;


	ASSERT_INT_EQ(prepare(&td), 0);

	r |= test_write(&td, "\033[7hbar", 1, td.vram.columns - 2, "bar", true);
	r |= test_write(&td, "\033[7lbar", 1, td.vram.columns - 2, "br", true);

	cleanup(&td);

	return -r;
}

TEST(vramtty_cursor){
	int r = 0;
	test_data_t td;


	ASSERT_INT_EQ(prepare(&td), 0);

	// simple escape codes
	r |= test_write(&td, "f\to", 0, 0, "f  o", false);
	r |= test_write(&td, "f\033Ho", 0, 0, "f  o", false);
	r |= test_write(&td, "foo\rbar", 0, 0, "bar", false);
	r |= test_write(&td, "foo\bbar", 0, 0, "fobar", false);
	r |= test_write(&td, "  \nfoo", 1, 0, "  foo", false);
	r |= test_write(&td, "  \vfoo", 1, 0, "  foo", false);
	r |= test_write(&td, "  \ffoo", 1, 0, "  foo", false);

	// set cursor position (\e[<line>;<columnH)
	r |= test_write(&td, "foo\n\033[HmvH", 0, 0, "mvH", false);
	r |= test_write(&td, "\033[3;2HmvH", 3, 2, "mvH", false);
	r |= test_write(&td, "\033[2;1fmvH", 2, 1, "mvH", false);

	// set column (\e[<n>G)
	r |= test_write(&td, "\033[3;2H\033[GmvG", 3, 0, "mvG", false);
	r |= test_write(&td, "\033[3;2H\033[4GmvG", 3, 4, "mvG", false);

	// up (\e[<n>A)
	r |= test_write(&td, "\033[3;2H\033[AmvA", 2, 2, "mvA", false);
	r |= test_write(&td, "\033[3;2H\033[2AmvA", 1, 2, "mvA", false);

	// up + start of line (\e[<n>F)
	r |= test_write(&td, "\033[3;2H\033[FmvF", 2, 0, "mvF", false);
	r |= test_write(&td, "\033[3;2H\033[2FmvF", 1, 0, "mvF", false);

	// up (\eM)
	r |= test_write(&td, "\033[3;2H\033MmvM", 2, 2, "mvM", false);

	// down (\e[<n>B)
	r |= test_write(&td, "\033[0;1H\033[BmvB", 1, 1, "mvB", false);
	r |= test_write(&td, "\033[0;1H\033[3BmvB", 3, 1, "mvB", false);

	// down + start of line (\e[<n>E)
	r |= test_write(&td, "\033[0;1H\033[EmvE", 1, 0, "mvE", false);
	r |= test_write(&td, "\033[0;1H\033[3EmvE", 3, 0, "mvE", false);

	// down (\eD)
	r |= test_write(&td, "\033[0;1H\033DmvD", 1, 1, "mvD", false);

	// right (\e[<n>C)
	r |= test_write(&td, "\033[1;1H\033[CmvC", 1, 2, "mvC", false);
	r |= test_write(&td, "\033[1;1H\033[3CmvC", 1, 4, "mvC", false);

	// left (\e[<n>D)
	r |= test_write(&td, "\033[2;4H\033[DmvD", 2, 3, "mvD", false);
	r |= test_write(&td, "\033[2;4H\033[3DmvD", 2, 1, "mvD", false);

	// save + restore
	r |= test_write(&td, " \nfoo\033[sbar", 1, 1, "foobar", false);
	r |= test_write(&td, "\033[ufoo", 1, 4, "foo", false);

	r |= test_write(&td, " \n\nfoo\0337bar", 2, 1, "foobar", false);
	r |= test_write(&td, "\0338foo", 2, 4, "foo", false);

	cleanup(&td);

	return -r;
}

TEST(vramtty_erase){
	int r = 0;
	test_data_t td;


	ASSERT_INT_EQ(prepare(&td), 0);

	r |= test_erase(&td, "\033[K", 1, 3, ERASE_LINE | ERASE_TO_END);
	r |= test_erase(&td, "\033[0K", 2, 2, ERASE_LINE | ERASE_TO_END);
	r |= test_erase(&td, "\033[1K", 2, 5, ERASE_LINE | ERASE_TO_START);
	r |= test_erase(&td, "\033[2K", 2, 5, ERASE_LINE | ERASE_TO_START | ERASE_TO_END);

	r |= test_erase(&td, "\033[J", 1, 3, ERASE_SCREEN | ERASE_TO_END);
	r |= test_erase(&td, "\033[0J", 2, 2, ERASE_SCREEN | ERASE_TO_END);
	r |= test_erase(&td, "\033[1J", 2, 5, ERASE_SCREEN | ERASE_TO_START);
	r |= test_erase(&td, "\033[2J", 2, 5, ERASE_SCREEN | ERASE_TO_START | ERASE_TO_END);

	cleanup(&td);

	return -r;
}

TEST(vramtty_scroll){
	int r = 0;
	test_data_t td;
	term_vram_cfg_t cfg;


	ASSERT_INT_EQ(prepare(&td), 0);
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGRD, &cfg), 0);

	// no scrolling since it is disabled
	r |= test_scroll(&td, "\033[A", 0);
	r |= test_scroll(&td, "\033[B", 0);

	// forced scrolling
	r |= test_scroll(&td, "\033M", -1);
	r |= test_scroll(&td, "\033D", 1);

	// scrolling since it is enabled
	cfg.term.lflags |= TLFL_SCROLL;
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGWR, &cfg), 0);

	r |= test_scroll(&td, "\033[2A", -2);
	r |= test_scroll(&td, "\033[2B", 2);

	cfg.term.lflags &= ~TLFL_SCROLL;
	r |= TEST_INT_EQ(ioctl(td.fd_term, IOCTL_CFGWR, &cfg), 0);

	cleanup(&td);

	return -r;
}

static int test_write(test_data_t *td, char *s, uint16_t line, uint16_t column, char *ref, bool move_cursor){
	int r = 0;


	TEST_LOG("write: %u,%u: %s\n", line, column, ref);

	r |= clearscreen(td);

	if(move_cursor)
		r |= move(td, line, column);

	r |= TEST_INT_EQ(write(td->fd_term, s, strlen(s)), strlen(s));
	r |= TEST_INT_EQ(writestr(td, line, column, ref), 0);

	r |= cmp(td);

	return -r;
}

static int test_erase(test_data_t *td, char *s, uint16_t line, uint16_t column, erase_t type){
	int r = 0;


	TEST_LOG("erase: %u,%u %x\n", line, column, type);

	r |= setscreen(td);

	// write to terminal
	r |= move(td, line, column);
	r |= TEST_INT_EQ(write(td->fd_term, s, strlen(s)), strlen(s));

	// erase from reference memory
	if((type & ERASE_SCREEN) && (type & ERASE_TO_START))
		memset(td->ref.ram, 0x0, line * td->ref.width);

	if((type & ERASE_SCREEN) && (type & ERASE_TO_END))
		memset(td->ref.ram + (line + 1) * td->ref.width, 0x0, (td->ref.lines - line) * td->ref.width);

	if(type & ERASE_TO_START)
		memset(td->ref.ram + line * td->ref.width, 0x0, column * td->font->width);

	if(type & ERASE_TO_END)
		memset(td->ref.ram + line * td->ref.width + column * td->font->width, 0x0, (td->ref.columns - column) * td->font->width);

	// compare
	r |= cmp(td);

	return -r;
}

static int test_scroll(test_data_t *td, char *s, int16_t lines){
	int r = 0;
	int16_t i;


	TEST_LOG("scroll: %d\n", lines);

	r |= setscreen(td);
	r |= move(td, (lines < 0) ? 0 : td->ref.lines - 1, td->ref.columns / 2);
	r |= TEST_INT_EQ(write(td->fd_term, s, strlen(s)), strlen(s));

	// scroll the reference memory
	for(i=(lines < 0 ? td->ref.lines-1 : 0); i>=0 && i<td->ref.lines; i+=(lines < 0 ? -1 : 1)){
		if(i + lines < 0 || i + lines >= td->ref.lines)
			memset(td->ref.ram + i * td->ref.width, 0x0, td->ref.width);
		else
			memcpy(td->ref.ram + i * td->ref.width, td->ref.ram + (i + lines) * td->ref.width, td->ref.width);
	}

	r |= cmp(td);

	return -r;
}

static int prepare(test_data_t *td){
	term_vram_cfg_t cfg;


	ASSERT_PTR_NEQ(td->font = font_resolve(0x0), 0x0);
	ASSERT_INT_NEQ(td->fd_loop = open(LOOP_DEV, O_RDWR), -1);
	ASSERT_INT_NEQ(td->fd_term = open(TERM_DEV, O_RDWR), -1);
	ASSERT_INT_EQ(ioctl(td->fd_term, IOCTL_CFGRD, &cfg), 0);

	cfg.term.lflags = TLFL_CANON;
	ASSERT_INT_EQ(ioctl(td->fd_term, IOCTL_CFGWR, &cfg), 0);

	td->refresh_ms = cfg.vram.refresh_ms;

	td->vram.height = cfg.vram.height;
	td->vram.width = cfg.vram.width;
	td->vram.size = MAX(1, td->vram.height / 8) * td->vram.width;
	td->vram.lines = cfg.term.lines;
	td->vram.columns = cfg.term.columns;

	td->ref = td->vram;
	td->ref.ram = calloc(1, td->ref.size);

	ASSERT_INT_EQ(td->vram.columns >= COLUMNS_MIN, true);
	ASSERT_INT_EQ(td->vram.lines >= LINES_MIN, true);
	ASSERT_PTR_NEQ(td->vram.ram = mmap(td->fd_loop, td->vram.size), 0x0);

	return 0;
}

static void cleanup(test_data_t *td){
	free(td->ref.ram);

	close(td->fd_loop);
	close(td->fd_term);
}

static int clearscreen(test_data_t *td){
	char cmd[] = "\033[H\033[2J";


	memset(td->ref.ram, 0x0, td->ref.size);

	return TEST_INT_EQ(write(td->fd_term, cmd, strlen(cmd)), strlen(cmd));
}

static int setscreen(test_data_t *td){
	int r = 0;
	char line[td->vram.columns];


	for(int16_t i=0; i<td->vram.lines; i++){
		memset(line, '0' + i, td->vram.columns);

		r |= move(td, i, 0);
		r |= TEST_INT_EQ(write(td->fd_term, line, td->vram.columns), td->vram.columns);

		for(int16_t j=0; j<td->vram.columns; j++)
			memcpy(td->ref.ram + i * td->ref.width + j * td->font->width, font_char('0' + i, td->font), td->font->width);
	}

	return -r;
}

static int writestr(test_data_t *td, int16_t line, int16_t column, char *s){
	for(int16_t i=0; s[i]!=0; i++){
		if((column + i) >= td->ref.columns){
			// line wrap
			line++;
			column = -i;

			if(line >= td->ref.lines)
				return -1;
		}

		memcpy(td->ref.ram + line * td->ref.width + (column + i) * td->font->width, font_char(s[i], td->font), td->font->width);
	}

	return 0;
}

static int cmp(test_data_t *td){
	sleep(td->refresh_ms, 0);

	return TEST_INT_EQ(memcmp(td->vram.ram, td->ref.ram, td->ref.size), 0);
}

static int move(test_data_t *td, uint16_t line, uint16_t column){
	char cmd[16];


	snprintf(cmd, 16, "\033[%d;%dH", line, column);

	return TEST_INT_EQ(write(td->fd_term, cmd, strlen(cmd)), strlen(cmd));
}
