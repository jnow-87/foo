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
#include <sys/compiler.h>
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

	uart_cfg_t cfg;
} dt_data_t;


/* local/static prototypes */
static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw);
static char putc(char c, void *hw);
static size_t putsn(char const *s, size_t n, void *hw);
static size_t gets(char *s, size_t n, void *hw);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	term_itf_t *itf;


	itf = kcalloc(1, sizeof(term_itf_t));

	if(itf == 0x0)
		return 0x0;

	itf->configure = configure;
	itf->putc = putc;
	itf->puts = putsn;
	itf->gets = gets;

	itf->hw = dtd;
	itf->cfg = &dtd->cfg;
	itf->cfg_size = sizeof(uart_cfg_t);
	itf->rx_int = dtd->rx_int;
	itf->tx_int = dtd->tx_int;

	return itf;
}

driver_probe("avr,uart", probe);

static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;
	uart_regs_t *regs = dtd->regs;
	uart_cfg_t *cfg = (uart_cfg_t*)hw_cfg;
	uint8_t const parity_bits[] = { 0b00, 0b11, 0b10 };
	unsigned int brate;


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

	return 0;
}

static char putc(char c, void *hw){
	uart_regs_t *regs = ((dt_data_t*)hw)->regs;


	while(!(regs->ucsra & (0x1 << UCSRA_UDRE)));
	regs->udr = c;

	return c;
}

static size_t putsn(char const *s, size_t n, void *hw){
	uart_regs_t *regs = ((dt_data_t*)hw)->regs;
	size_t i;


	if(s == 0x0)
		goto_errno(err, E_INVAL);

	for(i=0; i<n; i++, s++){
		while(!(regs->ucsra & (0x1 << UCSRA_UDRE)));
		regs->udr = *s;
	}

	return i;


err:
	return 0;
}

static size_t gets(char *s, size_t n, void *hw){
	uart_regs_t *regs = ((dt_data_t*)hw)->regs;
	size_t i = 0;


	/* read data */
	while(i < n && (regs->ucsra & (0x1 << UCSRA_RXC))){
		if(regs->ucsra & ((0x1 << UCSRA_FE) | (0x1 << UCSRA_DOR) | (0x1 << UCSRA_UPE)))
			goto_errno(err, E_IO);

		s[i] = regs->udr;
		i++;
	}

	return i;


err:
	return 0;
}
