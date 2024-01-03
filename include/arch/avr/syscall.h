/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_SYSCALL_H
#define AVR_SYSCALL_H


#include <sys/types.h>
#include <sys/syscall.h>


/* incomplete types */
struct thread_t;


/* prototypes */
int avr_sc(sc_num_t num, void *param, size_t psize);
sc_t *avr_sc_arg(struct thread_t *this_t);


#endif // AVR_SYSCALL_H
