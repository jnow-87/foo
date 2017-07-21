#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H


#include <sys/errno.h>


/* macros */
#define int_enable(mask)				(arch_kernel_call(int_enable, E_OK)(mask))
#define int_enabled()					(arch_kernel_call(int_enabled, INT_NONE)())
#define int_hdlr_register(num, hdlr)	(arch_kernel_call(int_hdlr_register, E_OK)(num, hdlr))
#define int_hdlr_release(num)			(arch_kernel_call(int_hdlr_release, E_OK)(num))


/* incomplete types */
enum int_num_t;


/* types */
typedef int (*int_hdlr_t)(enum int_num_t);


#include <arch/arch.h>
#include <sys/errno.h>


#endif // ARCH_INTERRUPT_H
