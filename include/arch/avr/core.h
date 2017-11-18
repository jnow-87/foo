#ifndef AVR_CORE_H
#define AVR_CORE_H


#include <arch/avr/thread.h>


/* prototypes */
void avr_core_sleep(void);

#ifdef BUILD_KERNEL
void avr_core_panic(thread_context_t const *tc);
#endif // BUILD_KERNEL


#endif // AVR_CORE_H
