/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_IPI_H
#define KERNEL_IPI_H


#include <sys/types.h>


/* types */
typedef void (*ipi_hdlr_t)(void *payload);

typedef struct ipi_msg_t{
	ipi_hdlr_t hdlr;
	uint8_t buf[];
} ipi_msg_t;


/* prototypes */
int ipi_send(unsigned int core, ipi_hdlr_t hdlr, void *buf, size_t n);


#endif // KERNEL_IPI_H
