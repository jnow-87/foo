#include <config/config.h>
#include <arch/arch.h>
#include <arch/avr/register.h>
#include <kernel/init.h>


/* macros */
#define BAUDRATE	((int)(((CONFIG_CORE_CLOCK_HZ / (16.0 * CONFIG_UART_BAUDRATE)) - 1) + 0.5))


/* global functions */
int avr_putchar(int c){
	while(!(mreg_r(UCSR0A) & (0x1 << UCSR0A_UDRE)));
	mreg_w(UDR0, c);

	return c;
}

int avr_puts(const char* s){
	if(s == 0)
		return -1;

	while(*s != 0){
		while(!(mreg_r(UCSR0A) & (0x1 << UCSR0A_UDRE)));
		mreg_w(UDR0, *s);
		s++;
	}

	return 0;
}


/* static functions */
static int avr_uart_init(void){
	if(BAUDRATE == 0)
		return -1;

	mreg_w(PRR0, (mreg_r(PRR0) & (-1 ^ (0x1 << PRR0_PRUSART0))));

	mreg_w(UBRR0H, (BAUDRATE >> 8));
	mreg_w(UBRR0L, (BAUDRATE & 0xff));

	mreg_w(UCSR0B,
		(0x1 << UCSR0B_RXEN) |
		(0x1 << UCSR0B_TXEN)
	);

	mreg_w(UCSR0C,
		(0x3 << UCSR0C_UCSZ0) |	// 8-bit character size

#if CONFIG_UART_PARITY_ODD
		(0x3 << UCSR0C_UPM0)
#elif CONFIG_UART_PARITY_EVEN
		(0x2 << UCSR0C_UPM0)
#else
		(0x0 << UCSR0C_UPM0)
#endif // CONFIG_UART_PARITY

		|

#if CONFIG_UART_STOPBITS == 1
		(0x0 << UCSR0C_USBS)
#elif CONFIG_UART_STOPBITS == 2
		(0x1 << UCSR0C_USBS)
#endif // CONFIG_UART_STOPBITS
	);

	return 0;
}

platform_init(1, avr_uart_init);
