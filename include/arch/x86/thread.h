/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_THREAD_H
#define X86_THREAD_H


/* types */
typedef struct thread_ctx_t{
	struct thread_ctx_t *next;
} thread_ctx_t;


#endif // X86_THREAD_H
