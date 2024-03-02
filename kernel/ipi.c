/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/devicetree.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/ipi.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>


/* local/static prototypes */
static void ipi_hdlr(int_num_t num, void *payload);


/* global functions */
int ipi_send(unsigned int core, ipi_hdlr_t hdlr, void *buf, size_t n){
	ipi_msg_t *msg;


	if(core >= DEVTREE_ARCH_NCORES)
		return_errno(E_INVAL);

	msg = kmalloc(sizeof(ipi_msg_t) + n);

	if(msg == 0x0)
		return -errno;

	msg->hdlr = hdlr;
	memcpy(msg->buf, buf, n);

	return ipi_int(core, false, msg);
}


/* local functions */
static int init(void){
	int r = 0;


	for(size_t i=0; i<DEVTREE_ARCH_NCORES; i++)
		r |= int_register(DEVTREE_ARCH_IPI_INT + i, ipi_hdlr, 0x0);

	return r;
}

kernel_init(0, init);

static void ipi_hdlr(int_num_t num, void *payload){
	ipi_msg_t *msg;


	msg = ipi_arg();

	if(msg != 0x0){
		msg->hdlr(msg->buf);
		kfree(msg);
	}
	else
		WARN("spurious ipi interrupt\n");
}
