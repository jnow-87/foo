/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_THREAD_H
#define ARM_THREAD_H


/* incomplete types */
struct thread_t;
enum thread_ctx_type_t;


/* types */
// NOTE when changing thread_ctx_t also check if modifications
// 		to the interrupt service routine are required
typedef struct thread_ctx_t{
	struct thread_ctx_t *next;

	/* TODO */
} thread_ctx_t;


#endif // ARM_THREAD_H
