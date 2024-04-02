/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <driver/term.h>
#include <sys/errno.h>
#include <sys/term.h>
#include <sys/types.h>
#include <sys/uart.h>


/* macros */
// register bits
#define UARTLCR_SPS			7
#define UARTLCR_WLEN		5
#define UARTLCR_FEN			4
#define UARTLCR_STP2		3
#define UARTLCR_EPS			2
#define UARTLCR_PEN			1
#define UARTLCR_BRK			0

#define UARTCR_CTSEN		15
#define UARTCR_RTSEN		14
#define UARTCR_OUT2			13
#define UARTCR_OUT1			12
#define UARTCR_RTS			11
#define UARTCR_DTR			10
#define UARTCR_RXE			9
#define UARTCR_TXE			8
#define UARTCR_LBE			7
#define UARTCR_SIRLP		2
#define UARTCR_SIREN		1
#define UARTCR_UARTEN		0

#define UARTFR_TXFF			5
#define UARTFR_RXFE			4

#define UARTIMSC_RTIM		6
#define UARTIMSC_TXIM		5
#define UARTIMSC_RXIM		4

#define UARTMIS_RTMIS		6
#define UARTMIS_TXMIS		5
#define UARTMIS_RXMIS		4

#define UARTIFLS_RXIFLSEL	3
#define UARTIFLS_TXIFLSEL	0

#define UARTICR_TXIC		5


/* types */
typedef struct{
	uint32_t volatile uartdr,
					  uartrsr,
					  padding0[4],
					  uartfr,
					  padding1,
					  uartilpr,
					  uartibrd,
					  uartfbrd,
					  uartlcr,
					  uartcr,
					  uartifls,
					  uartimsc,
					  uartris,
					  uartmis,
					  uarticr;
} uart_regs_t;

typedef struct{
	uart_regs_t *regs;
	uint8_t reset_id;	/**< cf. rp2040_resets_id_t */

	uint8_t hw_int,		// the hardware interrupt number
			vrx_int,	// virtual rx an tx interrupts to properly
			vtx_int;	// integrate with the terminal driver interface

	uart_cfg_t cfg;
} dt_data_t;


/* local/static prototypes */
static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw);
static size_t puts(char const *s, size_t n, bool blocking, void *hw);
static size_t gets(char *s, size_t n, void *hw);

static void int_hdlr(int_num_t num, void *payload);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd = (dt_data_t*)dt_data;
	term_itf_t *itf;


	if(!!dtd->hw_int != !!(dtd->vrx_int | dtd->vtx_int))
		goto_errno(err, E_INVAL);

	itf = kcalloc(1, sizeof(term_itf_t));

	if(itf == 0x0)
		goto err;

	itf->configure = configure;
	itf->puts = puts;
	itf->gets = gets;

	itf->hw = dtd;
	itf->cfg = &dtd->cfg;
	itf->cfg_size = sizeof(uart_cfg_t);
	itf->rx_int = dtd->vrx_int;
	itf->tx_int = dtd->vtx_int;

	return itf;


err:
	return 0x0;
}

driver_probe("rp2040,uart", probe);

static int configure(term_cfg_t *term_cfg, void *hw_cfg, void *hw){
	dt_data_t *dtd = (dt_data_t*)hw;
	uart_regs_t *regs = dtd->regs;
	uart_cfg_t *cfg = (uart_cfg_t*)hw_cfg;
	uint16_t brate;


	/* clear uart state */
	rp2040_resets_halt(0x1 << dtd->reset_id);
	rp2040_resets_release(0x1 << dtd->reset_id);

	regs->uartcr = 0x0;		// disable
	regs->uartrsr = 0xf;	// clear errors

	/* configure */
	// baudrate
	// according to the manual the baudrate registers are computed as
	//   IBRD.FBRD = uart_clock / (16 * baudrate)
	// the following computation is equivalent to IBRD.FBRD * 128, i.e.
	// shifted by 7 to avoid the need to deal with floats
	brate = (RP2040_PERI_CLOCK_HZ * 8) / cfg->baudrate;
	regs->uartibrd = brate >> 7;
	regs->uartfbrd = ((brate & 0x7f) + 1) >> 1;

	// fifo interrupt trigger levels
	// in case the fifos are enabled, cf. UARTLCR[FEN]
	regs->uartifls = (0x0 << UARTIFLS_RXIFLSEL)	// trigger rx: >= 1/8 full
				   | (0x4 << UARTIFLS_TXIFLSEL)	// trigger tx: <= 7/8 full
				   ;

	// line control
	regs->uartlcr = (0x0 << UARTLCR_SPS)
				  | (cfg->csize << UARTLCR_WLEN)
				  | (cfg->stopb << UARTLCR_STP2)
				  | ((cfg->parity ? 1 : 0) << UARTLCR_PEN)
				  | ((cfg->parity == UART_PARITY_EVEN ? 0x1 : 0x0) << UARTLCR_EPS)
				  | (0x1 << UARTLCR_FEN)
				  | (0x0 << UARTLCR_BRK)
				  ;

	// interrupts
	regs->uartimsc = ((dtd->vrx_int ? 0x1 : 0x0) << UARTIMSC_RXIM)
				   | ((dtd->vrx_int ? 0x1 : 0x0) << UARTIMSC_RTIM)
				   | ((dtd->vtx_int ? 0x1 : 0x0) << UARTIMSC_TXIM)
				   ;

	/* enable */
	regs->uartcr = (0x1 << UARTCR_UARTEN)
				 | (0x1 << UARTCR_RXE)
				 | (0x1 << UARTCR_TXE)
				 | (0x0 << UARTCR_CTSEN)
				 | (0x0 << UARTCR_RTSEN)
				 | (0x0 << UARTCR_OUT2)
				 | (0x0 << UARTCR_OUT1)
				 | (0x0 << UARTCR_RTS)
				 | (0x0 << UARTCR_DTR)
				 | (0x0 << UARTCR_SIRLP)
				 | (0x0 << UARTCR_SIREN)
				 | (0x0 << UARTCR_LBE)
				 ;

	if(dtd->hw_int == 0)
		return 0;

	av6m_nvic_int_enable(dtd->hw_int);

	return int_register(dtd->hw_int, int_hdlr, dtd);
}

static size_t puts(char const *s, size_t n, bool blocking, void *hw){
	uart_regs_t *regs = ((dt_data_t*)hw)->regs;
	size_t i;


	for(i=0; i<n; i++, s++){
		while(regs->uartfr & (0x1 << UARTFR_TXFF)){
			if(!blocking)
				return i;
		}

		regs->uartdr = *s;
	}

	return i;
}

static size_t gets(char *s, size_t n, void *hw){
	uart_regs_t *regs = ((dt_data_t*)hw)->regs;
	size_t i = 0;


	while(i < n && !(regs->uartfr & (0x1 << UARTFR_RXFE))){
		s[i++] = regs->uartdr & 0xff;

		if(regs->uartrsr & 0xf)
			goto_errno(err, E_IO);
	}

	return i;


err:
	regs->uartrsr = 0xf;	// clear errors

	return 0;
}

static void int_hdlr(int_num_t num, void *payload){
	dt_data_t *dtd = (dt_data_t*)payload;


	if(dtd->regs->uartmis & ((0x1 << UARTMIS_RXMIS) | (0x1 << UARTMIS_RTMIS)))
		int_foretell(dtd->vrx_int);

	if(dtd->regs->uartmis & (0x1 << UARTMIS_TXMIS)){
		dtd->regs->uarticr |= (0x1 << UARTICR_TXIC);
		int_foretell(dtd->vtx_int);
	}
}
