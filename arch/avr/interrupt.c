#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/core.h>
#include <arch/io.h>
#include <kernel/kprintf.h>
#include <sys/error.h>


/* external prototypes */
extern void __isr_reset(void);


/* local/static prototypes */
static void avr_int_inval(void);


/* global variables */
int_hdlr_t int_map[NINTERRUPTS] = { 0x0 };


/* global functions */
void avr_int_hdlr(isr_hdlr_t addr){
	int_num_t num;


	num = (addr - __isr_reset - INT_VEC_WORDS) / INT_VEC_WORDS;

	if(int_map[num] != 0)	int_map[num](num);
	else					avr_int_inval();
}

error_t avr_int_enable(int_type_t mask){
	if(mask)	asm volatile("sei");
	else		asm volatile("cli");

	return E_OK;
}

int_type_t avr_int_enabled(void){
	if(mreg_r(SREG) & (0x1 << SREG_I))
		return INT_GLOBAL;
	return INT_NONE;
}

error_t avr_int_hdlr_register(int_num_t num, int_hdlr_t hdlr){
	if(int_map[num] != 0x0){
		WARN("interrupt already registerd %u %#x\n", num, int_map[num]);
		return E_INUSE;
	}

	int_map[num] = hdlr;

	return E_OK;
}

error_t avr_int_hdlr_release(int_num_t num){
	int_map[num] = 0x0;
	return E_OK;
}


/* local functions */
static void avr_int_inval(void){
	puts("invalid isr\n");
	core_halt();
}
