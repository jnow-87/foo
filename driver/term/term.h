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
int term_cursor_set(term_t *term, uint16_t line, uint16_t column);
int term_cursor_move(term_t *term, int16_t lines, int16_t columns, bool force_scroll);
void term_cursor_save(term_t *term);
int term_cursor_restore(term_t *term);
int term_cursor_show(term_t *term, bool show);

int term_esc_handle(term_t *term, char c);

void term_esc_reset(term_t *term);
bool term_esc_active(term_t *term);


#endif // TERM_H
