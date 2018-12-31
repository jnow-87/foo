/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/avr/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/opt.h>
#include <kernel/ksignal.h>
#include <kernel/init.h>
#include <driver/term.h>
#include <sys/term.h>
#include <sys/errno.h>
#include <sys/register.h>
#include <sys/compiler.h>
#include <sys/mutex.h>



#if (CONFIG_AVR_UART_CNT > UART_CNT)
	CPP_ERROR(uart count violation - avr target device supports utmost UART_CNT uart(s))
#endif // CONFIG_AVR_UART_CNT


/* local/static prototypes */
static int config(term_t *term, term_cfg_t *cfg);
static int putsn(term_t *term, char const *s, size_t n);
static void rx_hdlr(uint8_t uart);


/* static variables */
static uint8_t volatile * const ucsra[] = { (uint8_t*)UCSR0A, (uint8_t*)UCSR1A },
						* const ucsrb[] = { (uint8_t*)UCSR0B, (uint8_t*)UCSR1B },
						* const ucsrc[] = { (uint8_t*)UCSR0C, (uint8_t*)UCSR1C },
						* const ubrrl[] = { (uint8_t*)UBRR0L, (uint8_t*)UBRR1L },
						* const ubrrh[] = { (uint8_t*)UBRR0H, (uint8_t*)UBRR1H },
						* const udr[] = { (uint8_t*)UDR0, (uint8_t*)UDR1 };

static term_t *terms[CONFIG_AVR_UART_CNT];
static mutex_t wr_mtx[CONFIG_AVR_UART_CNT];


/* global functions */
char avr_uart_putchar(char c){
	mutex_lock(wr_mtx);

	while(!((*(ucsra[0])) & (0x1 << UCSR0A_UDRE)));
	*(udr[0]) = c;

	mutex_unlock(wr_mtx);

	return c;
}

int avr_uart_puts(char const *s){
	if(s == 0)
		return_errno(E_INVAL);

	for(; *s!=0; s++)
		avr_uart_putchar(*s);

	return E_OK;
}


/* local functions */
static int kuart_init(void){
	unsigned int i;
	term_t term;


	/* apply kernel uart config */
	for(i=0; i<CONFIG_AVR_UART_CNT; i++){
		term.data = (void*)i;

		if(config(&term, &kopt.term_cfg) != E_OK)
			return -errno;

		mutex_init(wr_mtx + i, MTX_NONE);
	}

	return E_OK;
}

platform_init(0, kuart_init);

static int init(void){
	unsigned int i;
	char suffix[] = "x";
	term_ops_t ops;


	ops.config = config;
	ops.puts = putsn;

	/* register terminal devices */
	for(i=0; i<CONFIG_AVR_UART_CNT; i++){
		suffix[0] = '0' + i;

		terms[i] = term_register(suffix, &ops, (void*)i);

		if(terms[i] == 0x0)
			return -errno;

		// NOTE default configuration has already been
		// 		applied through kuart_init()
		terms[i]->cfg = kopt.term_cfg;
	}

	return E_OK;
}

driver_init(init);

static int config(term_t *term, term_cfg_t *cfg){
	static uint8_t const parity_bits[] = { 0b00, 0b11, 0b10 };
	static uint8_t const stopb_bits[] = { 0b0, 0b1 };
	static uint8_t const prr_bits[] = { PRR0_PRUSART0, PRR0_PRUSART1 };
	unsigned int brate;
	unsigned int uart;


	uart = (unsigned int)term->data;

	if(cfg->baud > 115200)
		return_errno(E_LIMIT);

	brate = (CONFIG_CORE_CLOCK_HZ / (cfg->baud * 16));

	if(brate == 0)
		return_errno(E_INVAL);

	/* finish ongoing transmissions */
	if(((*(ucsrb[uart])) & UCSR0B_TXEN)){
		mutex_lock(wr_mtx + uart);
		while(!((*(ucsra[uart])) & (0x1 << UCSR0A_TXC)));
		mutex_unlock(wr_mtx + uart);
	}

	/* disable uart, triggering reset */
	mreg_w(PRR0, mreg_r(PRR0) | (0x1 << prr_bits[uart]));

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

static int putsn(term_t *term, char const *s, size_t n){
	unsigned int uart;


	uart = (unsigned int)term->data;


	if(s == 0)
		return_errno(E_INVAL);

	mutex_lock(wr_mtx + uart);

	for(; n>0; n--, s++){
		while(!(*(ucsra[uart]) & (0x1 << UCSR0A_UDRE)));
		*(udr[uart]) = *s;
	}

	mutex_unlock(wr_mtx + uart);

	return E_OK;
}

#if (CONFIG_AVR_UART_CNT > 0)

static void uart0_rx_hdlr(void){
	rx_hdlr(0);
}

avr_int(INT_USART0_RX, uart0_rx_hdlr);

#endif // CONFIG_AVR_UART_CNT

#if (CONFIG_AVR_UART_CNT > 1)

static void uart1_rx_hdlr(void){
	rx_hdlr(1);
}

avr_int(INT_USART1_RX, uart1_rx_hdlr);

#endif // CONFIG_AVR_UART_CNT

static void rx_hdlr(uint8_t uart){
	uint8_t c,
			err;


	err = 0;

	/* read data */
	while((*(ucsra[uart]) & (0x1 << UCSR0A_RXC))){
		err |= *(ucsra[uart]) & ((0x1 << UCSR0A_FE) | (0x1 << UCSR0A_DOR) | (0x1 << UCSR0A_UPE));
		c = *(udr[uart]);

		if(err)
			continue;

		if(ringbuf_write(&terms[uart]->rx_buf, &c, 1) != 1)
			err |= (0x1 << UCSR0A_RXC);	// use UCSR0A non-error flag to signal buffer overrun
	}

	/* update error */
	if(err){
		terms[uart]->rx_err |= (bits(err, UCSR0A_FE, 0x1) ? TE_FRAME : 0)
							|  (bits(err, UCSR0A_DOR, 0x1) ? TE_DATA_OVERRUN : 0)
							|  (bits(err, UCSR0A_UPE, 0x1) ? TE_PARITY : 0)
							|  (bits(err, UCSR0A_RXC, 0x1) ? TE_RX_FULL : 0)
							;
	}

	ksignal_send(terms[uart]->rx_rdy);
}
