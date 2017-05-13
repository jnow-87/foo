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
 * \brief	set bit synchronously
 * 			this sequence is intented for bits that require a change enable flag
 * 			in order to allow changing the actual bit
 *
 * \param	reg_addr		address of the register to modify
 * \param	bit				bit that shall be modified
 * \param	chg_en			change enable bit
 */
#define mreg_bit_set_sync(reg_addr, bit, chg_en){ \
	register uint8_t t; \
	\
	\
	asm volatile( \
		"lds	%[tmp], %[addr]\n" \
		"ori	%[tmp], %[val_en]\n" \
		"sts	%[addr], %[tmp]\n" \
		"ori	%[tmp], %[val_reg]\n" \
		"andi	%[tmp], %[val_di]\n" \
		"sts	%[addr], %[tmp]\n" \
		: [tmp] "=r" (t) \
		: [addr] "i" (reg_addr), \
		  [val_en] "i" (0x1 << chg_en), \
		  [val_di] "i" (0xff ^ (0x1 << chg_en)), \
		  [val_reg] "i" (0x1 << bit) \
	); \
}

/**
 * \brief	clear bit synchronously
 * 			this sequence is intented for bits that require a change enable flag
 * 			in order to allow changing the actual bit
 *
 * \param	reg_addr		address of the register to modify
 * \param	bit				bit that shall be modified
 * \param	chg_en			change enable bit
 */
#define mreg_bit_clr_sync(reg_addr, bit, chg_en){ \
	register uint8_t t; \
	\
	\
	asm volatile( \
		"lds	%[tmp], %[addr]\n" \
		"ori	%[tmp], %[val_en]\n" \
		"sts	%[addr], %[tmp]\n" \
		"andi	%[tmp], %[val_di]\n" \
		"sts	%[addr], %[tmp]\n" \
		: [tmp] "=r" (t) \
		: [addr] "i" (reg_addr), \
		  [val_en] "i" (0x1 << chg_en), \
		  [val_di] "i" (0xff ^ ((0x1 << chg_en) | (0x1 << bit))) \
	); \
}


#endif // AVR_REGISTER_H
