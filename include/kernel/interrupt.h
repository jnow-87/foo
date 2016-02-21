#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H


/* incomplete types */
enum int_num_t;


/* types */
typedef int(*int_hdlr_t)(enum int_num_t);


#endif // KERNEL_INTERRUPT_H
