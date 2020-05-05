/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_CRITSEC_H
#define KERNEL_CRITSEC_H


#include <arch/interrupt.h>
#include <sys/mutex.h>


/* macros */
#define CRITSEC_INITIALISER(){ \
	.imask = 0, \
	.mtx = MUTEX_INITIALISER(), \
}


/* types */
typedef struct{
	mutex_t mtx;
	int_type_t imask;
} critsec_lock_t;


/* prototypes */
void critsec_init(critsec_lock_t *l);
void critsec_lock(critsec_lock_t *l);
void critsec_unlock(critsec_lock_t *l);


#endif // KERNEL_CRITSEC_H
