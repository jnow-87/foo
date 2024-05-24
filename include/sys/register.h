/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef REGISTER_H
#define REGISTER_H


#include <sys/types.h>


/* macros */
#define MREG(addr)	(*((register_t volatile*)(addr)))

#define bits_get(val, idx, mask)({ \
	typeof(val) _val = val; \
	typeof(idx) _idx = idx; \
	typeof(val) _mask = mask; \
	\
	\
	(((_val) & (_mask << (_idx))) >> (_idx)); \
})

#define bits_set(val, mask)({ \
	typeof(mask) _mask = mask; \
	\
	\
	((val & ~_mask) | _mask); \
})

#define lo8(val)	bits_get((unsigned int)(val), 0, 0xff)
#define hi8(val)	bits_get((unsigned int)(val), 8, 0xff)
#define hh8(val)	bits_get((unsigned int)(val), 16, 0xff)


/* prototypes */
uint8_t bits_count(unsigned int val);
int8_t bits_highest(unsigned int val);


#endif // REGISTER_H
