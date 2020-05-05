/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <kernel/critsec.h>
#include <sys/mutex.h>


/* global functions */
void critsec_init(critsec_lock_t *l){
	mutex_init(&l->mtx, MTX_NONE);
	l->imask = 0;
}

void critsec_lock(critsec_lock_t *l){
	l->imask = int_enable(INT_NONE);
	mutex_lock(&l->mtx);
}

void critsec_unlock(critsec_lock_t *l){
	mutex_unlock(&l->mtx);
	int_enable(l->imask);
}
