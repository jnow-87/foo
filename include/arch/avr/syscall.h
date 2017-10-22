#ifndef AVR_SYSCALL_H
#define AVR_SYSCALL_H


#include <sys/types.h>
#include <sys/syscall.h>


/* prototypes */
void avr_sc(sc_t num, void *param, size_t psize);
int avr_sc_hdlr(void);


#endif // AVR_SYSCALL_H
