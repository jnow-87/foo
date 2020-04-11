/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/avr/register.h>
#include <kernel/driver.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <driver/term.h>
#include <sys/register.h>
#include <sys/term.h>
#include <sys/spi.h>
#include <sys/errno.h>


/* macros */
// register bits
#define SPCR_SPIE		7
#define SPCR_SPE		6
#define SPCR_DORD		5
#define SPCR_MSTR		4
#define SPCR_CPOL		3
#define SPCR_CPHA		2
#define SPCR_SPR		0

#define SPSR_SPIF		7
#define SPSR_WCOL		6
#define SPSR_SPI2X		0


/* types */
typedef struct{
	uint8_t volatile spcr,
					 spsr,
					 spdr;
} spi_regs_t;

typedef struct{
	// device registers
	spi_regs_t *regs;

	// port
	uint8_t volatile *ddr;
	uint8_t ddr_miso,			// bit mask
			ddr_mosi,			// bit mask
			ddr_sck;			// bit mask

	// power control
	uint8_t volatile *prr;
	uint8_t const prr_en;		// PRR device enable value (bit mask)

	// interrupt
	uint8_t const int_num;
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
	itf->rx_int = dtd->int_num;
	itf->tx_int = 0;
	itf->cfg_size = sizeof(spi_cfg_t);

	return itf;
}

interface_probe("avr,spi", probe);

static int configure(void *_cfg, void *data){
	uint8_t const pres_bits[] = { 0b100, 0b000, 0b101, 0b001, 0b110, 0b010, 0b011 };
	spi_cfg_t *cfg;
	dt_data_t *dtd;
	spi_regs_t *regs;


	dtd = (dt_data_t*)data;
	regs = dtd->regs;
	cfg = (spi_cfg_t*)_cfg;

	/* disable spi, triggering reset */
	regs->spcr = 0x0;
	*dtd->prr |= dtd->prr_en;

	/* enable spi */
	// power control
	*dtd->prr &= ~dtd->prr_en;

	// pin congiguration
	if(cfg->dev_mode == SPI_DEV_MASTER)	*dtd->ddr |= dtd->ddr_mosi | dtd->ddr_sck;
	else								*dtd->ddr |= dtd->ddr_miso;

	// interface configuration
	regs->spsr = (((pres_bits[cfg->prescaler] & 0x4) >> 2) << SPSR_SPI2X);
	regs->spcr = (0x1 << SPCR_SPE)
			   | ((dtd->int_num ? 0x1 : 0x0) << SPCR_SPIE)
			   | (cfg->dev_mode << SPCR_MSTR)
			   | (cfg->data_order << SPCR_DORD)
			   | ((cfg->sample_mode >= SPI_SAMPLE_MODE_2) << SPCR_CPOL)
			   | ((cfg->sample_mode % 2) << SPCR_CPHA)
			   | ((pres_bits[cfg->prescaler] & 0x3) << SPCR_SPR)
			   ;

	return E_OK;
}

static term_flags_t get_flags(void *cfg){
	return ((spi_cfg_t*)cfg)->flags;
}

static char putc(char c, void *data){
	spi_regs_t *regs;


	regs = ((dt_data_t*)data)->regs;

	regs->spdr = c;
	while(!(regs->spsr & (0x1 << SPSR_SPIF)));

	DEBUG("%c (%#hhx)\n", c, c);

	return c;
}

static size_t putsn(char const *s, size_t n, void *data){
	size_t i;
	spi_regs_t *regs;


	regs = ((dt_data_t*)data)->regs;

	if(s == 0x0){
		errno = E_INVAL;
		return 0;
	}

	for(i=0; i<n; i++, s++){
		regs->spdr = *s;
		while(!(regs->spsr & (0x1 << SPSR_SPIF)));

		DEBUG("send %c (%#hhx)\n", *s, *s);
	}

	return i;
}

static size_t gets(char *s, size_t n, term_err_t *err, void *data){
	spi_regs_t *regs;


	regs = ((dt_data_t*)data)->regs;

	if(n == 0)
		return 0;

	*s = regs->spdr;

	if(regs->spsr & (0x1 << SPSR_WCOL)){
		*err |= TERM_ERR_WRITE_COLL;
		DEBUG("rx error, read %c (%#x)\n", *s, (int)*s);

		return 0;
	}

	DEBUG("read %c (%#x)\n", *s, (int)*s);

	return 1;
}
