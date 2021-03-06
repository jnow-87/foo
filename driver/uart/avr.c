/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/driver.h>
#include <driver/term.h>
#include <sys/register.h>
#include <sys/term.h>
#include <sys/uart.h>
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
	uint8_t volatile ucsra,
					 ucsrb,
					 ucsrc;

	uint8_t res0;

	uint8_t volatile ubrrl,
					 ubrrh;

	uint8_t volatile udr;
} uart_regs_t;

typedef struct{
	// device registers
	uart_regs_t *regs;

	// power control
	uint8_t volatile *prr;
	uint8_t const prr_en;		// PRR device enable value (bit mask)

	// interrupt number
	uint8_t const rx_int,
				  tx_int;
} dt_data_t;


/* local/static prototypes */
static int configure(void *cfg, void *data);
static term_flags_t get_flags(void *cfg);
static char putc(char c, void *data);
static size_t putsn(char const *s, size_t n, void *data);
static size_t gets(char *s, size_t n, term_err_t *err, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd;
	term_itf_t *itf;


	dtd = (dt_data_t*)dt_data;
	itf = kmalloc(sizeof(term_itf_t));

	if(itf == 0x0)
		return 0x0;

	itf->configure = configure;
	itf->get_flags = get_flags;
	itf->putc = putc;
	itf->puts = putsn;
	itf->gets = gets;
	itf->data = dtd;
	itf->rx_int = dtd->rx_int;
	itf->tx_int = dtd->tx_int;
	itf->cfg_size = sizeof(uart_cfg_t);

	return itf;
}

interface_probe("avr,uart", probe);

static int configure(void *_cfg, void *data){
	uint8_t const parity_bits[] = { 0b00, 0b11, 0b10 };
	unsigned int brate;
	dt_data_t *dtd;
	uart_regs_t *regs;
	uart_cfg_t *cfg;


	dtd = (dt_data_t*)data;
	regs = dtd->regs;
	cfg = (uart_cfg_t*)_cfg;

	/* compute baud rate */
	if(cfg->baudrate > 115200)
		return_errno(E_LIMIT);

	brate = (AVR_IO_CLOCK_HZ / (cfg->baudrate * 16));

	if(brate == 0)
		return_errno(E_INVAL);

	/* finish ongoing transmissions */
	if(regs->ucsrb & UCSRB_TXEN){
		while(!(regs->ucsra & (0x1 << UCSRA_TXC)));
	}

	/* disable uart, triggering reset */
	regs->ucsrb = 0x0;
	*dtd->prr |= dtd->prr_en;

	/* re-enable uart */
	*dtd->prr &= ~dtd->prr_en;

	regs->ucsra = 0x0 << UCSRA_U2X;
	regs->ucsrb = (0x1 << UCSRB_RXEN)
				| (0x1 << UCSRB_TXEN)
				| ((dtd->rx_int ? 0x1 : 0x0) << UCSRB_RXCIE)
				| ((dtd->tx_int ? 0x1 : 0x0) << UCSRB_TXCIE)
				;

	// set baudrate
	regs->ubrrh = hi8(brate);
	regs->ubrrl = lo8(brate);

	// set csize, parity, stop bits
	regs->ucsrc = (cfg->csize << UCSRC_UCSZ)
				| (parity_bits[cfg->parity] << UCSRC_UPM)
				| (cfg->stopb << UCSRC_USBS)
				;

	return E_OK;
}

static term_flags_t get_flags(void *cfg){
	return ((uart_cfg_t*)cfg)->flags;
}

static char putc(char c, void *data){
	uart_regs_t *regs;


	regs = ((dt_data_t*)data)->regs;

	while(!(regs->ucsra & (0x1 << UCSRA_UDRE)));
	regs->udr = c;

	return c;
}

static size_t putsn(char const *s, size_t n, void *data){
	size_t i;
	uart_regs_t *regs;


	regs = ((dt_data_t*)data)->regs;

	if(s == 0x0){
		errno = E_INVAL;
		return 0;
	}

	for(i=0; i<n; i++, s++){
		while(!(regs->ucsra & (0x1 << UCSRA_UDRE)));
		regs->udr = *s;
	}

	return i;
}

static size_t gets(char *s, size_t n, term_err_t *err, void *data){
	size_t i;
	uint8_t e;
	uart_regs_t *regs;


	regs = ((dt_data_t*)data)->regs;
	e = 0;

	/* read data */
	i = 0;

	while(i < n && (regs->ucsra & (0x1 << UCSRA_RXC))){
		e |= regs->ucsra & ((0x1 << UCSRA_FE) | (0x1 << UCSRA_DOR) | (0x1 << UCSRA_UPE));
		s[i] = regs->udr;

		if(e){
			*err |= (bits(e, UCSRA_FE, 0x1) ? TERM_ERR_FRAME : 0)
				 |  (bits(e, UCSRA_DOR, 0x1) ? TERM_ERR_DATA_OVERRUN : 0)
				 |  (bits(e, UCSRA_UPE, 0x1) ? TERM_ERR_PARITY : 0)
				 |  (bits(e, UCSRA_RXC, 0x1) ? TERM_ERR_RX_FULL : 0)
				 ;
		}

		i++;
	}

	return i;
}
