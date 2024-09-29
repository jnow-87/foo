/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TERM_H
#define TERM_H


#include <driver/term.h>
#include <sys/types.h>


/* prototypes */
size_t puts_raw(term_t *term, char const *s, size_t n);

char *flags_apply(term_t *term, char *s, size_t n, size_t incr, term_flag_type_t fl_type, uint8_t flags);

int cursor_set(term_t *term, uint16_t line, uint16_t column);
int cursor_move(term_t *term, int16_t lines, int16_t columns, bool force_scroll);
void cursor_save(term_t *term);
int cursor_restore(term_t *term);
int cursor_show(term_t *term, bool show);

int esc_handle(term_t *term, char c);


#endif // TERM_H
