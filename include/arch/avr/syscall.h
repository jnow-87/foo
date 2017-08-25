#ifndef AVR_SYSCALL_H
#define AVR_SYSCALL_H


#include <sys/types.h>
#include <sys/syscall.h>


/* prototypes */
int avr_sc(sc_t num, void *param, size_t psize);


#endif // AVR_SYSCALL_H
