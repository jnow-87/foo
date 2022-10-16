/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef REGISTER_H
#define REGISTER_H


/* macros */
#define mreg_bits(reg_addr, idx, mask) \
	bits(mreg_r(reg_addr), idx, mask)

#define bits(val, idx, mask)({ \
	typeof(val) _val = val; \
	typeof(idx) _idx = idx; \
	typeof(val) _mask = mask; \
	\
	\
	(((_val) & (_mask << (_idx))) >> (_idx)); \
})

#define lo8(val)	bits((unsigned int)(val), 0, 0xff)
#define hi8(val)	bits((unsigned int)(val), 8, 0xff)
#define hh8(val)	bits((unsigned int)(val), 16, 0xff)


#endif // REGISTER_H
