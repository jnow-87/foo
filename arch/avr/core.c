void avr_core_sleep(void){
	asm volatile("sleep");
}

void avr_core_halt(void){
	asm volatile(
		"break\n"
		"sleep\n"
	);
}
