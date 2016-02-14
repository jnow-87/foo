/* macros */
#define NISR	35


/* global variables */
int (*isr_hdlr[NISR])(void) = { 0x0 };
