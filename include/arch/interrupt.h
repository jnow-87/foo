#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H


#include <arch/arch.h>


/* macros */
#define int_enable(num)					arch_kernel_call(int_enable, 0)(num)
#define int_disable(num)				arch_kernel_call(int_disable, 0)(num)
#define int_enabled()					arch_kernel_call(int_enabled, 0)()
#define int_hdlr_register(num, hdlr)	arch_kernel_call(int_hdlr_register(num, hdlr)
#define int_hdlr_release(num)			arch_kernel_call(int_hdlr_release(num)


#endif // ARCH_INTERRUPT_H
