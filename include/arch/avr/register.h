/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



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

#define mreg_w(reg_addr, reg_val) \
	asm volatile( \
		"sts	%0, %1\n" \
		: \
		: "i" (reg_addr), "r" ((uint8_t)(reg_val)) \
	)

/**
 * \brief	set register synchronously
 * 			this sequence is intented for bits that require a change enable flag
 * 			in order to allow changing the actual bit
 *
 * \param	reg_addr		address of the register to modify
 * \param	reg_val			intended value
 * \param	chg_en			change enable bit
 *
 * \pre		interrupts are assumed to be disabled, otherwise the following sequence might fail
 */
#define mreg_w_sync(reg_addr, reg_val, chg_en){ \
	register uint8_t en = (0x1 << chg_en); \
	\
	\
	asm volatile( \
		"sts	%[addr], %[en]\n" \
		"sts	%[addr], %[val]\n" \
		: \
		: [addr] "i" (reg_addr), \
		  [en] "r" (en), \
		  [val] "r" ((uint8_t)(reg_val & (0xff ^ (0x1 << chg_en)))) \
	); \
}


#endif // AVR_REGISTER_H
