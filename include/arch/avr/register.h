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
	);

#define mreg_bset_sync(reg_addr, bit_c, bit_c_en){ \
	register uint8_t t; \
	\
	\
	asm volatile( \
		"lds	%[tmp], %[addr]\n" \
		"ori	%[tmp], %[v_en]\n" \
		"sts	%[addr], %[tmp]\n" \
		"ori	%[tmp], %[v_new]\n" \
		"andi	%[tmp], %[v_di]\n" \
		"sts	%[addr], %[tmp]\n" \
		: [tmp] "=r" (t) \
		: [addr] "i" (reg_addr), \
		  [v_en] "i" (0x1 << bit_c_en), \
		  [v_di] "i" (0xff ^ (0x1 << bit_c_en)), \
		  [v_new] "i" (0x1 << bit_c) \
	); \
}

#define mreg_bclr_sync(reg_addr, bit_c, bit_c_en){ \
	register uint8_t t; \
	\
	\
	asm volatile( \
		"lds	%[tmp], %[addr]\n" \
		"ori	%[tmp], %[v_en]\n" \
		"sts	%[addr], %[tmp]\n" \
		"andi	%[tmp], %[v_di]\n" \
		"sts	%[addr], %[tmp]\n" \
		: [tmp] "=r" (t) \
		: [addr] "i" (reg_addr), \
		  [v_en] "i" (0x1 << bit_c_en), \
		  [v_di] "i" (0xff ^ ((0x1 << bit_c_en) | (0x1 << bit_c))) \
	); \
}


#endif // AVR_REGISTER_H
