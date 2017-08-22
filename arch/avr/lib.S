#include <arch/arch.h>


.extern main
.extern exit


.global avr_libmain
avr_libmain:
	ICALL main
	ICALL exit
