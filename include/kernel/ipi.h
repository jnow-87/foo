/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_IPI_H
#define KERNEL_IPI_H


/* types */
typedef void (*ipi_hdlr_t)(void *data);


/* prototypes */
int ipi_send(unsigned int core, ipi_hdlr_t hdlr, void *data, size_t size);
void kipi_hdlr(void);


#endif // KERNEL_IPI_H
