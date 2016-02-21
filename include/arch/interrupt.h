#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H


#include <arch/arch.h>


/* macros */
#define int_enable(mask)				arch_kernel_call(int_enable, 0)(mask)
#define int_get_mask()					arch_kernel_call(int_get_mask, 0)()
#define int_hdlr_register(num, hdlr)	arch_kernel_call(int_hdlr_register(num, hdlr)
#define int_hdlr_release(num)			arch_kernel_call(int_hdlr_release(num)


#endif // ARCH_INTERRUPT_H
