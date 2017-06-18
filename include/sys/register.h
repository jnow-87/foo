#ifndef REGISTER_H
#define REGISTER_H


/* macros */
#define mreg_bits(reg_addr, idx, mask) \
	bits(mreg_r(reg_addr), idx, mask)

#define bits(val, idx, mask) \
	(((val) & ((mask) << (idx))) >> (idx))

#define lo8(val)	bits((unsigned int)(val), 0, 0xff)
#define hi8(val)	bits((unsigned int)(val), 8, 0xff)
#define hh8(val)	bits((unsigned int)(val), 16, 0xff)


#endif // REGISTER_H
