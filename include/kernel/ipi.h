/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_IPI_H
#define KERNEL_IPI_H


/* types */
typedef enum{
	IPI0 = 0x1,
	IPI1,
	IPI2,
	IPI3,
	IPI_MAX
} ipi_t;


#endif // KERNEL_IPI_H
