#ifndef AVR_CORE_H
#define AVR_CORE_H


/* prototypes */
void avr_core_sleep(void);

#ifdef BUILD_KERNEL
void avr_core_panic(void);
#endif // BUILD_KERNEL


#endif // AVR_CORE_H
