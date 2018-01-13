#include <arch/syscall.h>
#include <sys/time.h>
#include <sys/syscall.h>


/* global functions */
int time(time_t *t){
	sc_time_t p;


	sc(SC_TIME, &p);

	*t = p.time;
	errno |= p.errno;

	return -p.errno;
}
