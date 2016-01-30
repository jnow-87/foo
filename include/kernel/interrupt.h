#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H


#include <sys/compiler.h>


/* types */
typedef enum __packed{
	  INT0 = 0x1,
	  INT1 = 0x2,
	  INT2 = 0x4,
	  INT3 = 0x8,
	  INT4 = 0x10,
	  INT5 = 0x20,
	  INT6 = 0x40,
	  INT7 = 0x80,
	  INT8 = 0x100,
	  INT9 = 0x200,
	 INT10 = 0x400,
	 INT11 = 0x800,
	 INT12 = 0x1000,
	 INT13 = 0x2000,
	 INT14 = 0x4000,
	 INT15 = 0x8000,
	 INT16 = 0x10000,
	 INT17 = 0x80000000,
	 INT18 = 0x80000000,
	 INT19 = 0x80000000,
	 INT20 = 0x80000000,
	 INT21 = 0x80000000,
	 INT22 = 0x80000000,
	 INT23 = 0x80000000,
	 INT24 = 0x80000000,
	 INT25 = 0x80000000,
	 INT26 = 0x80000000,
	 INT27 = 0x80000000,
	 INT28 = 0x80000000,
	 INT29 = 0x80000000,
	 INT30 = 0x80000000,
	 INT31 = 0x80000000,
	 INT32 = 0x80000000,
	INTALL = 0xffffffff,
} int_num_t;

typedef int(*int_hdlr_t)(int_num_t);


#endif // KERNEL_INTERRUPT_H
