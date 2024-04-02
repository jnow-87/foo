/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/v6m.h>
#include <kernel/interrupt.h>
#include <sys/types.h>


/* macros */
// registers
#define ISER	0xe100
#define ICER	0xe180
#define ICPR	0xe280
#define IPR(x)	(0xe400 + x * 4)


/* global functions */
void av6m_nvic_init(void){
	ppb_write(ICER, 0xffffffff);	// disable all interrupts
	ppb_write(ICPR, 0xffffffff);	// clear pending interrupts
}

void av6m_nvic_int_enable(int_num_t num){
	ppb_write(ICPR, (0x1 << (num - INT_EXT_BASE)));
	ppb_write(ISER, (0x1 << (num - INT_EXT_BASE)));
}

void av6m_nvic_int_disable(int_num_t num){
	ppb_write(ICER, (0x1 << (num - INT_EXT_BASE)));
}

void av6m_nvic_int_prio_set(uint8_t num, uint8_t prio){
	uint8_t reg = num / 4;
	uint8_t bits = 6 + (num - reg * 4) * 8;


	ppb_write(IPR(reg), ((ppb_read(IPR(reg)) & (~((uint32_t)0x3 << bits))) | ((prio & 0x3) << bits)));
}
