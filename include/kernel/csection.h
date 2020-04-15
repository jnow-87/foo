/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_CSECTION_H
#define KERNEL_CSECTION_H


#include <arch/interrupt.h>
#include <sys/mutex.h>


/* macros */
#define CSECTION_INITIALISER(){ \
	.imask = 0, \
	.mtx = MUTEX_INITIALISER(), \
}


/* types */
typedef struct{
	mutex_t mtx;
	int_type_t imask;
} csection_lock_t;


/* prototypes */
void csection_init(csection_lock_t *l);
void csection_lock(csection_lock_t *l);
void csection_unlock(csection_lock_t *l);


#endif // KERNEL_CSECTION_H
