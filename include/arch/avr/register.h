#ifndef AVR_REGISTER_H
#define AVR_REGISTER_H


/* macros */
#define mreg_r(reg_addr)({ \
	register register_t val; \
	\
	\
	asm volatile( \
		"lds	%0, %1\n" \
		: "=r" (val) \
		: "i" (reg_addr) \
	); \
	\
	val; \
})

#define mreg_w(reg_addr, reg_val)	\
	asm volatile( \
		"sts	%0, %1\n" \
		: \
		: "i" (reg_addr), "r" (reg_val) \
	);


#endif // AVR_REGISTER_H
