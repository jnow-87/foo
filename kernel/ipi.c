/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/core.h>
#include <arch/interrupt.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stack.h>
#include <sys/mutex.h>
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
	char data[];
} ipi_msg_t;


/* static variables */
static ipi_msg_t *messages[CONFIG_NCORES] = { 0x0 };
static mutex_t msg_mtx = MUTEX_INITIALISER();


/* global functions */
int ipi_send(unsigned int core, ipi_hdlr_t hdlr, void *data, size_t size){
	ipi_msg_t *msg;


	if(core >= CONFIG_NCORES)
		return_errno(E_INVAL);

	msg = kmalloc(sizeof(ipi_msg_t) + size);

	if(msg == 0x0)
		return_errno(E_NOMEM);

	msg->hdlr = hdlr;
	memcpy(msg->data, data, size);

	mutex_lock(&msg_mtx);
	stack_push(messages[core], msg);
	mutex_unlock(&msg_mtx);

	int_ipi(core, false);

	return 0;
}

void kipi_hdlr(void){
	ipi_msg_t *msg;


	msg = stack_pop(messages[PIR]);
	msg->hdlr(msg->data);
	kfree(msg);
}
