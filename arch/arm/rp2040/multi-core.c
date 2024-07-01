/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <arch/arm/v6m.h>
#include <kernel/init.h>
#include <kernel/ipi.h>
#include <kernel/memory.h>
#include <sys/compiler.h>
#include <sys/devicetree.h>
#include <sys/register.h>
#include <sys/types.h>


/* macros */
#define FIFO_RD		MREG(SIO_BASE | 0x058)
#define FIFO_WR		MREG(SIO_BASE | 0x054)
#define FIFO_ST		MREG(SIO_BASE | 0x050)

#define FIFO_ST_ROE	3
#define FIFO_ST_WOF	2
#define FIFO_ST_RDY	1
#define FIFO_ST_VLD	0


/* external prototypes */
extern void __av6m_start_multi_core(void);


/* local/static prototypes */
static uint32_t fifo_read(void);
static void fifo_write(uint32_t v);
static void fifo_clear(void);


/* global functions */
int rp2040_ipi_int(unsigned int core, bool bcast, ipi_msg_t *msg){
	fifo_write((uint32_t)msg);

	return 0;
}

ipi_msg_t *rp2040_ipi_arg(void){
	return (ipi_msg_t*)fifo_read();
}

void rp2040_cores_boot(void){
	uint32_t cmds[] = {
		0,
		0,
		1,
		ppb_read(VTOR),						// interrupt vector table
		(uint32_t)KERNEL_STACK(1),			// stack pointer
		(uint32_t)__av6m_start_multi_core,	// entry point
	};
	uint8_t i = 0;


	av6m_nvic_int_disable(DEVTREE_ARCH_IPI_INT);

	while(i < sizeof_array(cmds)){
		if(cmds[i] == 0){
			fifo_clear();
			asm volatile("sev");
		}

		fifo_write(cmds[i]);
		i = (fifo_read() == cmds[i]) ? i + 1 : 0;
	}

	av6m_nvic_int_enable(DEVTREE_ARCH_IPI_INT);
}


/* local functions */
static int init(void){
	FIFO_ST = 0x0;
	fifo_clear();

	av6m_nvic_int_enable(DEVTREE_ARCH_IPI_INT + PIR);

	return 0;
}

platform_init(1, all, init);

static uint32_t fifo_read(void){
	while((FIFO_ST & (0x1 << FIFO_ST_VLD)) == 0){
		asm volatile("wfe");
	}

	return FIFO_RD;
}

static void fifo_write(uint32_t v){
	while((FIFO_ST & (0x1 << FIFO_ST_RDY)) == 0);

	FIFO_WR = v;
	asm volatile("sev");
}

static void fifo_clear(void){
	while(FIFO_ST & (0x1 << FIFO_ST_VLD)){
		(void)FIFO_RD;
	}
}
