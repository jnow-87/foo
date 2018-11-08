/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <lib/unistd.h>
#include <sys/syscall.h>


/* global functions */
tid_t thread_create(int (*entry)(void *), void *arg){
	sc_thread_t p;


	p.entry = entry;
	p.arg = arg;

	if(sc(SC_THREADCREATE, &p) != E_OK)
		return 0;
	return p.tid;
}

int thread_info(thread_info_t *info){
	int r;
	sc_thread_t p;


	r = sc(SC_THREADINFO, &p);

	info->tid = p.tid;
	info->priority = p.priority;
	info->affinity = p.affinity;

	return r;
}

int nice(int inc){
	int r;
	sc_thread_t p;


	p.priority = inc;
	r = sc(SC_NICE, &p);

	if(r != E_OK)
		return r;
	return p.priority;
}
