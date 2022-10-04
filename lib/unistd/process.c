/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <lib/unistd.h>
#include <sys/syscall.h>
#include <sys/binloader.h>
#include <sys/string.h>


/* global functions */
pid_t process_create(void *binary, bin_type_t bin_type, char const *name, char const *args){
	sc_process_t p;


	p.binary = binary;
	p.bin_type = bin_type;
	p.name = name;
	p.name_len = strlen(name);
	p.args = args;
	p.args_len = strlen(args);

	if(sc(SC_PROCCREATE, &p) != E_OK)
		return 0;

	return p.pid;
}

int process_info(process_info_t *info){
	int r;
	sc_process_t p;


	r = sc(SC_PROCINFO, &p);

	info->pid = p.pid;

	return r;
}
