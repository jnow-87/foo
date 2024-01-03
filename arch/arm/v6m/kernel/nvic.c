#include <arch/arm/v6m.h>
#include <kernel/interrupt.h>
#include <sys/types.h>
#include <sys/register.h>


/* macros */
#define ISER	MREG(AV6M_PPB_BASE + 0xe100)
#define ICER	MREG(AV6M_PPB_BASE + 0xe180)
#define ICPR	MREG(AV6M_PPB_BASE + 0xe280)
#define IPR(x)	MREG(AV6M_PPB_BASE + 0xe400 + x * 4)


/* global functions */
void av6m_nvic_init(void){
	ICER = 0xffffffff;	// disable all interrupts
	ICPR = 0xffffffff;	// clear pending interrupts
}

void av6m_nvic_int_enable(int_num_t num){
	ICPR = (0x1 << (num - INT_EXT_BASE));
	ISER = (0x1 << (num - INT_EXT_BASE));
}

void av6m_nvic_int_disable(int_num_t num){
	ICER = (0x1 << (num - INT_EXT_BASE));
}

void av6m_nvic_int_prio_set(uint8_t num, uint8_t prio){
	uint8_t reg = num / 4;
	uint8_t bits = 6 + (num - reg * 4) * 8;


	IPR(reg) = (IPR(reg) & (~((uint32_t)0x3 << bits))) | ((prio & 0x3) << bits);
}
