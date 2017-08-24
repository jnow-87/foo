#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H


#include <sys/types.h>
#include <sys/syscall.h>


/* types */
typedef int (*syscall_hdlr_t)(void *param);


/* prototypes */
int syscall_register(syscall_t num, syscall_hdlr_t hdlr);
int syscall_release(syscall_t num);

int ksyscall_hdlr(syscall_t num, void *param, size_t psize);


#endif // KERNEL_SYSCALL_H
