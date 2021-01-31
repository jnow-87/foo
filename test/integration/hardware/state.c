/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <pthread.h>
#include <hardware/hardware.h>


/* global variables */
hw_state_t hw_state = {
	.priviledge = HWS_KERNEL,
	.int_enabled = false,
	.syscall_pending = false,
};


/* static variables */
static pthread_mutex_t hw_state_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t hw_state_changed = PTHREAD_COND_INITIALIZER;


/* global functions */
void hw_state_lock(void){
	pthread_mutex_lock(&hw_state_mtx);
}

void hw_state_unlock(void){
	pthread_cond_signal(&hw_state_changed);
	pthread_mutex_unlock(&hw_state_mtx);
}

void hw_state_wait(void){
	pthread_cond_wait(&hw_state_changed, &hw_state_mtx);
}
