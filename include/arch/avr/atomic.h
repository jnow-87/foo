#ifndef AVR_ATOMIC_H
#define AVR_ATOMIC_H


/* prototypes */
int avr_cas(int volatile *v, int old, int new);


#endif // AVR_ATOMIC_H
