/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ctxsanity_H
#define ctxsanity_H


#include <kernel/thread.h>


/* prototypes */
int ctxsanity_check_all_ctx(char const *msg, int int_num, int sc);
int ctxsanity_check_ctx(char const *msg, int int_num, int sc, thread_ctx_t *tc, thread_ctx_t *tc_cmp, thread_t const *this_t_entry, thread_t const *this_t_exit);
int ctxsanity_check_stack_addr(char const *msg, int int_num, int sc, struct thread_ctx_t *tc, thread_t const *this_t);
int ctxsanity_sc_num(void);


#endif // ctxsanity_H
