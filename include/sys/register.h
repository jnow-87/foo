#ifndef REGISTER_H
#define REGISTER_H


#include <arch/arch.h>
#include <sys/types.h>


/* macros */
#define mregbits(reg_addr, idx, mask) \
	bits(regr(reg_addr), idx, mask)

#define bits(val, idx, mask) \
	(((val) & ((mask) << (idx))) >> (idx))


#endif // REGISTER_H
