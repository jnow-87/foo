/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <pthread.h>
#include <sys/escape.h>
#include <hardware/hardware.h>
#include <user/opts.h>


/* global variables */
hw_state_t hw_state = {
	.privilege = HWS_KERNEL,
	.tid = 0,
	.int_enabled = false,
	.locked = false,
	.syscall_pending = false,
	.stats = { 0 },
};


/* static variables */
static pthread_mutex_t hw_state_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t hw_state_changed = PTHREAD_COND_INITIALIZER;


/* global functions */
void hw_state_lock(void){
	pthread_mutex_lock(&hw_state_mtx);

	hw_state.locked = true;
	hw_state_print();
}

void hw_state_unlock(void){
	pthread_cond_signal(&hw_state_changed);
	pthread_mutex_unlock(&hw_state_mtx);

	hw_state.locked = false;
	hw_state_print();
}

void hw_state_wait(void){
	pthread_cond_wait(&hw_state_changed, &hw_state_mtx);
}

void hw_state_print(void){
	if(opts.stats_fd < 0)
		return;

	dprintf(opts.stats_fd,
		CLEAR
		SET_POS
		"events ack:  %zu\n"
		"evants nack: %zu\n"
		"\n"
		"int ack:  %zu\n"
		"int nack: %zu\n"
		"\n"
		"privilege: %s\n"
		"tid: %u\n"
		"locked: %d\n"
		,
		1, 1,
		hw_state.stats.event_ack,
		hw_state.stats.event_nack,
		hw_state.stats.int_ack,
		hw_state.stats.int_nack,
		(hw_state.privilege == HWS_KERNEL) ? "kernel" : "user",
		hw_state.tid,
		hw_state.locked
	);
}
