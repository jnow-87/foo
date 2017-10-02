#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <driver/uart.h>
#include <driver/avr_uart.h>
#include <sys/uart.h>
#include <sys/errno.h>
#include <sys/register.h>


/* macros */
#if (CONFIG_NUM_UART > 2)
	#error "avr uart driver only supports up to 2 UARTS"
#endif // CONFIG_NUM_UART


/* local/static prototypes */
static int rx_hdlr(int_num_t num);


/* global varibales */
uart_cbs_t uart_cbs = {
	.config = avr_uart_config,
	.puts = avr_uart_putsn,
};


/* static variables */
static int_num_t const rx_int_num[] = { INT_USART0_RX, INT_USART1_RX };
static uint8_t volatile * const ucsra[] = { (uint8_t*)UCSR0A, (uint8_t*)UCSR1A },
						* const ucsrb[] = { (uint8_t*)UCSR0B, (uint8_t*)UCSR1B },
						* const ucsrc[] = { (uint8_t*)UCSR0C, (uint8_t*)UCSR1C },
						* const ubrrl[] = { (uint8_t*)UBRR0L, (uint8_t*)UBRR1L },
						* const ubrrh[] = { (uint8_t*)UBRR0H, (uint8_t*)UBRR1H },
						* const udr[] = { (uint8_t*)UDR0, (uint8_t*)UDR1 };



/* global functions */
int avr_uart_config(unsigned int uart, uart_t *cfg){
	static uint8_t const parity_bits[] = { 0b00, 0b11, 0b10 };
	static uint8_t const stopb_bits[] = { 0b0, 0b1 };
	static uint8_t const prr_bits[] = { PRR0_PRUSART0, PRR0_PRUSART1 };
	unsigned int brate;


	if(cfg->baud > 115200)
		return_errno(E_LIMIT);

	brate = (CONFIG_CORE_CLOCK_HZ / (cfg->baud * 16));

	if(brate == 0)
		return_errno(E_INVAL);

	/* register interrupt handler */
	if(int_hdlr_register(rx_int_num[uart], rx_hdlr) != E_OK)
		return errno;

	/* enable uart */
	mreg_w(PRR0, (mreg_r(PRR0) & (-1 ^ (0x1 << prr_bits[uart]))));

	*(ucsra[uart]) = 0x0 << UCSR0A_U2X;
	*(ucsrb[uart]) = (0x1 << UCSR0B_RXEN) |
					 (0x1 << UCSR0B_TXEN) |
					 (0x1 << UCSR0B_RXCIE);

	/* set baudrate */
	*(ubrrh[uart]) = hi8(brate);
	*(ubrrl[uart]) = lo8(brate);

	/* set csize, parity, stop bits */
	*(ucsrc[uart]) = (cfg->csize << UCSR0C_UCSZ0) |
					 (parity_bits[cfg->parity] << UCSR0C_UPM0) |
					 (stopb_bits[cfg->stopb] << UCSR0C_USBS);

	return E_OK;
}

char avr_uart_putchar(char c){
	while(!((*(ucsra[0])) & (0x1 << UCSR0A_UDRE)));
	*(udr[0]) = c;

	return c;
}

int avr_uart_puts(char const *s){
	if(s == 0)
		return_errno(E_INVAL);

	for(; *s!=0; s++){
		while(!((*(ucsra[0])) & (0x1 << UCSR0A_UDRE)));
		*(udr[0]) = *s;
	}

	return E_OK;
}

int avr_uart_putsn(unsigned int uart, char const *s, size_t n){
	if(s == 0)
		return_errno(E_INVAL);

	for(; n>0; n--, s++){
		while(!(*(ucsra[uart]) & (0x1 << UCSR0A_UDRE)));
		*(udr[uart]) = *s;
	}

	return E_OK;
}


/* static functions */
static int rx_hdlr(int_num_t num){
	unsigned int uart;
	uint8_t c,
			err;


	err = 0;

	/* get uart number */
	for(uart=0; uart<CONFIG_NUM_UART; uart++){
		if(rx_int_num[uart] == num)
			break;
	}

	/* read data */
	while((*(ucsra[uart]) & (0x1 << UCSR0A_RXC))){
		err |= *(ucsra[uart]) & ((0x1 << UCSR0A_FE) | (0x1 << UCSR0A_DOR) | (0x1 << UCSR0A_UPE));
		c = *(udr[uart]);

		if(err)
			continue;

		if(ringbuf_write(uart_rx_buf + uart, &c, 1) != 1)
			err |= (0x1 << UCSR0A_RXC);	// use UCSR0A non-error flag to signal buffer overrun
	}

	/* update error */
	if(err){
		uart_rx_err[uart] |= err;

		uart_cfg[uart].frame_err |= bits(err, UCSR0A_FE, 0x1);
		uart_cfg[uart].data_overrun |= bits(err, UCSR0A_DOR, 0x1);
		uart_cfg[uart].parity_err |= bits(err, UCSR0A_UPE, 0x1);
		uart_cfg[uart].rx_queue_full |= bits(err, UCSR0A_RXC, 0x1);
	}

	return E_OK;
}
