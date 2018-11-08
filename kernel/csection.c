/*
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <kernel/csection.h>
#include <sys/mutex.h>


/* global functions */
void csection_init(csection_lock_t *l){
	mutex_init(&l->mtx, 0);
	l->imask = 0;
}

void csection_lock(csection_lock_t *l){
	l->imask = int_enable(INT_NONE);
	mutex_lock(&l->mtx);
}

void csection_unlock(csection_lock_t *l){
	mutex_unlock(&l->mtx);
	int_enable(l->imask);
}
