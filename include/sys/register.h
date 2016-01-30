#ifndef REGISTER_H
#define REGISTER_H


#include <sys/types.h>


/* macros */
#define mregr(reg_addr)({ \
	register register_t val; \
	\
	\
	asm volatile( \
		"lds	%0, %1;" \
		: "=r" (val) \
		: "i" (reg_addr) \
	); \
	\
	val; \
})

#define mregw(reg_addr, reg_val)	\
	asm volatile( \
		"sts	%0, %1;" \
		: \
		: "i" (reg_addr), "r" (reg_val) \
	);

#define mregbits(reg_addr, idx, mask) \
	bits(regr(reg_addr), idx, mask)

#define bits(val, idx, mask) \
	(((val) & ((mask) << (idx))) >> (idx))


#endif // REGISTER_H
