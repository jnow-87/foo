#include <arch/avr/interrupt.h>


/* global variables */
int (*isr_hdlr[NINTERRUPTS])(void) = { 0x0 };
