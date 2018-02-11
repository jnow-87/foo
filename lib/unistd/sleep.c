#include <arch/syscall.h>
#include <sys/syscall.h>
#include <sys/types.h>


/* global functions */
int sleep(size_t ms, size_t us){
	sc_time_t p;


	p.time.us = us;
	p.time.ms = ms;

	return sc(SC_SLEEP, &p);
}
