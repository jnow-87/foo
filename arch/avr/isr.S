/* macros */
#define isr(name, tgt) \
.global __isr_##name $ \
__isr_##name: \
	rcall tgt


/* interrupt vectors */
.section .reset

isr(reset, __start)
isr(int0, __isr_call)
isr(int1, __isr_call)
isr(int2, __isr_call)
isr(pcint0, __isr_call)
isr(pcint1, __isr_call)
isr(pcint2, __isr_call)
isr(pcint3, __isr_call)
isr(watchdog, __isr_call)
isr(timer2_comp_a, __isr_call)
isr(timer2_comp_b, __isr_call)
isr(timer2_ovfl, __isr_call)
isr(timer1_capture, __isr_call)
isr(timer1_comp_a, __isr_call)
isr(timer1_comp_b, __isr_call)
isr(timer1_ovfl, __isr_call)
isr(timer0_comp_a, __isr_call)
isr(timer0_comp_b, __isr_call)
isr(timer0_ovfl, __isr_call)
isr(spi_tx, __isr_call)
isr(usart0_rx, __isr_call)
isr(usart0_udre, __isr_call)
isr(usart0_tx, __isr_call)
isr(analog_comp, __isr_call)
isr(adc, __isr_call)
isr(eeprom_rdy, __isr_call)
isr(twi, __isr_call)
isr(spm_rdy, __isr_call)
isr(usart1_rx, __isr_call)
isr(usart1_udre, __isr_call)
isr(usart1_tx, __isr_call)
isr(timer3_capture, __isr_call)
isr(timer3_comp_a, __isr_call)
isr(timer3_comp_b, __isr_call)
isr(timer3_ovfl, __isr_call)
