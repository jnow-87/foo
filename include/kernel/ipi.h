#ifndef KERNEL_IPI_H
#define KERNEL_IPI_H


#include <sys/compiler.h>


/* types */
typedef enum __packed{
	IPI0 = 0x1,
	IPI1,
	IPI2,
	IPI3,
	IPI_MAX
} ipi_t;


#endif // KERNEL_IPI_H
