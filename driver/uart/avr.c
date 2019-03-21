/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/opt.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/driver.h>
#include <driver/term.h>
#include <sys/register.h>
#include <sys/term.h>
#include <sys/errno.h>


/* macros */
// register bits
#define UCSRA_RXC		7
#define UCSRA_TXC		6
#define UCSRA_UDRE		5
#define UCSRA_FE		4
#define UCSRA_DOR		3
#define UCSRA_UPE		2
#define UCSRA_U2X		1
#define UCSRA_MPCM		0

#define UCSRB_RXCIE		7
#define UCSRB_TXCIE		6
#define UCSRB_UDRIE		5
#define UCSRB_RXEN		4
#define UCSRB_TXEN		3
#define UCSRB_UCSZ2		2
#define UCSRB_RXB8		1
#define UCSRB_TXB8		0

#define UCSRC_UMSEL1	7
#define UCSRC_UMSEL0	6
#define UCSRC_UPM1		5
#define UCSRC_UPM		4
#define UCSRC_USBS		3
#define UCSRC_UCSZ1		2
#define UCSRC_UCSZ		1
#define UCSRC_UCPOL		0


/* types */
typedef struct{
	// config registers */
	uint8_t volatile ucsra,
					 ucsrb,
					 ucsrc;

	uint8_t res0;

	// baud rate registers
	uint8_t volatile ubrrl,
					 ubrrh;

	// data register
	uint8_t volatile udr;
} uart_reg_t;

typedef struct{
	uart_reg_t *uart;

	// power control
	uint8_t volatile *prr;
	uint8_t const prr_mask;

	// interrupt number
	uint8_t rx_int,
			tx_int;
} uart_devtree_t;


/* local/static prototypes */
static int configure(term_cfg_t *cfg, void *regs);
static size_t putsn(char const *s, size_t n, void *regs);
static size_t gets(char *s, size_t n, term_err_t *err, void *regs);


/* global functions */
char avr_uart_putchar(char c){
	while(!(mreg_r(UCSR0A) & (0x1 << UCSRA_UDRE)));
	mreg_w(UDR0, c);

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
	uart_devtree_t regs = {
		.uart = (void*)UCSR0A,
		.prr = (void*)PRR0,
		.prr_mask = (0x1 << PRR0_PRUSART0),
		.rx_int = 0,
		.tx_int = 0,
	};


	/* apply kernel uart config */
	(void)configure(&kopt.term_cfg, &regs);
	return -errno;
}

platform_init(0, kuart_init);

static void *probe(void *dt_data, void *dt_itf){
	uart_devtree_t *regs;
	term_itf_t *itf;


	regs = (uart_devtree_t*)dt_data;
	itf = kmalloc(sizeof(term_itf_t));

	if(itf == 0x0)
		return 0x0;

	itf->configure = configure;
	itf->puts = putsn;
	itf->gets = gets;
	itf->rx_int = regs->rx_int;
	itf->tx_int = regs->tx_int;
	itf->regs = regs;

	return itf;
}

driver_interface("avr,uart", probe);

static int configure(term_cfg_t *cfg, void *_regs){
	uint8_t const parity_bits[] = { 0b00, 0b11, 0b10 };
	unsigned int brate;
	uart_devtree_t *regs;


	regs = (uart_devtree_t*)_regs;

	/* compute baud rate */
	if(cfg->baud > 115200)
		return_errno(E_LIMIT);

	brate = (CONFIG_CORE_CLOCK_HZ / (cfg->baud * 16));

	if(brate == 0)
		return_errno(E_INVAL);

	/* finish ongoing transmissions */
	if(regs->uart->ucsrb & UCSRB_TXEN){
		while(!(regs->uart->ucsra & (0x1 << UCSRA_TXC)));
	}

	/* disable uart, triggering reset */
	*regs->prr |= regs->prr_mask;

	/* re-enable uart */
	*regs->prr &= (-1 ^ regs->prr_mask);

	regs->uart->ucsra = 0x0 << UCSRA_U2X;
	regs->uart->ucsrb = (0x1 << UCSRB_RXEN)
					  | (0x1 << UCSRB_TXEN)
					  | ((regs->rx_int ? 0x1 : 0x0) << UCSRB_RXCIE)
					  | ((regs->tx_int ? 0x1 : 0x0) << UCSRB_TXCIE)
					  ;

	// set baudrate
	regs->uart->ubrrh = hi8(brate);
	regs->uart->ubrrl = lo8(brate);

	// set csize, parity, stop bits
	regs->uart->ucsrc = (cfg->csize << UCSRC_UCSZ)
					  | (parity_bits[cfg->parity] << UCSRC_UPM)
					  | (cfg->stopb << UCSRC_USBS)
					  ;

	return E_OK;
}

static size_t putsn(char const *s, size_t n, void *_regs){
	size_t i;
	uart_devtree_t *regs;


	regs = (uart_devtree_t*)_regs;

	if(s == 0x0){
		errno = E_INVAL;
		return 0;
	}

	for(i=0; i<n; i++, s++){
		while(!(regs->uart->ucsra & (0x1 << UCSRA_UDRE)));
		regs->uart->udr = *s;
	}

	return i;
}

static size_t gets(char *s, size_t n, term_err_t *err, void *_regs){
	size_t i;
	uint8_t e;
	uart_devtree_t *regs;


	regs = (uart_devtree_t*)_regs;
	e = 0;

	/* read data */
	i = 0;

	while(i < n && (regs->uart->ucsra & (0x1 << UCSRA_RXC))){
		e |= regs->uart->ucsra & ((0x1 << UCSRA_FE) | (0x1 << UCSRA_DOR) | (0x1 << UCSRA_UPE));
		s[i] = regs->uart->udr;

		if(e){
			*err |= (bits(e, UCSRA_FE, 0x1) ? TE_FRAME : 0)
				 |  (bits(e, UCSRA_DOR, 0x1) ? TE_DATA_OVERRUN : 0)
				 |  (bits(e, UCSRA_UPE, 0x1) ? TE_PARITY : 0)
				 |  (bits(e, UCSRA_RXC, 0x1) ? TE_RX_FULL : 0)
				 ;
		}
		else
			i++;
	}

	return i;
}
