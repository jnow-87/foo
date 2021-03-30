/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/process.h>
#include <kernel/binloader.h>
#include <sys/errno.h>
#include <sys/thread.h>


/* global functions */
int bin_load_raw(void *binary, process_t *this_p){
	this_p->entry = (process_entry_t)binary;

	return E_OK;
}
