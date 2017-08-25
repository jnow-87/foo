#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H


#include <sys/types.h>
#include <sys/syscall.h>


/* types */
typedef int (*sc_hdlr_t)(void *param);


/* prototypes */
int sc_register(sc_t num, sc_hdlr_t hdlr);
int sc_release(sc_t num);

int ksc_hdlr(sc_t num, void *param, size_t psize);


#endif // KERNEL_SYSCALL_H
