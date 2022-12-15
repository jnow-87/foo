/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/core.h>
#include <arch/interrupt.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/stack.h>
#include <sys/mutex.h>
#include <sys/devicetree.h>
#include <kernel/ipi.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>

/**
 * TODO
 * 	the functions in this file need to be tested as soon
 * 	as multi-core targets are implemented
 */


/* types */
typedef struct ipi_msg_t{
	struct ipi_msg_t *next;

	ipi_hdlr_t hdlr;
	uint8_t buf[];
} ipi_msg_t;


/* static variables */
static ipi_msg_t *messages[DEVTREE_ARCH_NCORES] = { 0x0 };
static mutex_t msg_mtx = MUTEX_INITIALISER();


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

	mutex_lock(&msg_mtx);
	stack_push(messages[core], msg);
	mutex_unlock(&msg_mtx);

	int_ipi(core, false);

	return 0;
}

void ipi_khdlr(void){
	ipi_msg_t *msg;


	msg = stack_pop(messages[PIR]);
	msg->hdlr(msg->buf);
	kfree(msg);
}
