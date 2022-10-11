/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/avr/atmega.h>
#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/register.h>
#include <sys/ioctl.h>
#include <sys/pwm.h>


/* macros */
// register bits
#define TCCRA_COMA	6
#define TCCRA_COMB	4
#define TCCRA_WGM0	0

#define TCCRB_ICNC	7
#define TCCRB_ICES	6
#define TCCRB_WGM3	4
#define TCCRB_WGM2	3
#define TCCRB_CS	0

#define TCCRC_FOCA	7
#define TCCRC_FOCB	6


/* types */
typedef struct{
	uint8_t volatile tccra,
					 tccrb,
					 tccrc;

	uint8_t res0;

	uint16_t volatile tcnt,
					  icr,
					  ocra,
					  ocrb;
} pwm_regs_t;

typedef struct{
	// device registers
	pwm_regs_t *regs;

	// interrupt control
	uint8_t volatile *tifr,
					 *timsk;

	// power control
	uint8_t volatile *prr;
	uint8_t const prr_en;		// PRR device enable value

	// i/o port control
	uint8_t volatile *ddr;
	uint8_t const ddr_en;		// DDR output compare enable value
} dt_data_t;


/* local/static prototypes */
static size_t read_a(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write_a(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t read_b(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write_b(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t read(unsigned int *buf, size_t n, uint16_t volatile *ocr_reg);
static size_t write(unsigned int *buf, size_t n, uint16_t volatile *ocr_reg);
static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n);
static int config_set(pwm_cfg_t *cfg, dt_data_t *dtd);
static int config_get(pwm_cfg_t *cfg, pwm_regs_t *regs);


/* local functions */
static void *probe(char const *_name, void *dt_data, void *dt_itf){
	size_t name_len = strlen(_name);
	char name[name_len + 2];
	devfs_dev_t *dev;
	devfs_ops_t ops;
	pwm_cfg_t cfg;


	/* configure timer0 */
	cfg.mode = PWM_FAST;
	cfg.base_clock_khz = 20000;
	cfg.prescaler = PWM_PRES_0;

	if(config_set(&cfg, dt_data) != 0)
		goto err_0;

	/* register device */
	strcpy(name, _name);
	name[name_len + 1] = 0;

	ops.open = 0x0;
	ops.close = 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;
	ops.mmap = 0x0;

	name[name_len] = 'a';
	ops.read = read_a;
	ops.write = write_a;

	if((dev = devfs_dev_register(name, &ops, dt_data)) == 0x0)
		goto err_0;

	name[name_len] = 'b';
	ops.read = read_b;
	ops.write = write_b;

	if(devfs_dev_register(name, &ops, dt_data) == 0x0)
		goto err_1;

	return 0x0;

err_1:
	devfs_dev_release(dev);

err_0:
	return 0x0;
}

driver_probe("avr,pwm16", probe);

static size_t read_a(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return read(buf, n, &((dt_data_t*)dev->payload)->regs->ocra);
}

static size_t write_a(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return write(buf, n, &((dt_data_t*)dev->payload)->regs->ocra);
}

static size_t read_b(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return read(buf, n, &((dt_data_t*)dev->payload)->regs->ocrb);
}

static size_t write_b(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return write(buf, n, &((dt_data_t*)dev->payload)->regs->ocrb);
}

static size_t read(unsigned int *buf, size_t n, uint16_t volatile *ocr_reg){
	if(n != sizeof(*buf)){
		set_errno(E_INVAL);

		return 0;
	}

	*buf = *ocr_reg;

	return sizeof(*buf);
}

static size_t write(unsigned int *buf, size_t n, uint16_t volatile *ocr_reg){
	if(n != sizeof(*buf)){
		set_errno(E_INVAL);

		return 0;
	}

	*ocr_reg = (*buf) & 0xffff;

	return sizeof(*buf);
}

static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *arg, size_t n){
	if(n != sizeof(pwm_cfg_t))
		return_errno(E_INVAL);

	switch(request){
	case IOCTL_CFGRD:	return config_get(arg, ((dt_data_t*)dev->payload)->regs);
	case IOCTL_CFGWR:	return config_set(arg, dev->payload);
	default:			return_errno(E_NOSUP);
	}
}

static int config_set(pwm_cfg_t *cfg, dt_data_t *dtd){
	pwm_regs_t *regs = dtd->regs;
	uint8_t pres;


	/* check config */
	if(cfg->mode != PWM_FAST && cfg->mode != PWM_PHASECORRECT)
		return_errno(E_NOSUP);

	switch(cfg->prescaler){
	case PWM_PRES_0:	pres = 0; break;
	case PWM_PRES_1:	pres = 1; break;
	case PWM_PRES_8:	pres = 2; break;
	case PWM_PRES_64:	pres = 3; break;
	case PWM_PRES_256:	pres = 4; break;
	case PWM_PRES_1024:	pres = 5; break;
	default:			return_errno(E_NOSUP);
	}

	/* make output compare pin an output */
	*dtd->ddr |= dtd->ddr_en;

	/* enable timer */
	*dtd->prr &= ~dtd->prr_en;

	/* set mode */
	regs->tccra = (0x2 << TCCRA_COMA)
				| (0x2 << TCCRA_COMB)
				| (0x3 << TCCRA_WGM0)
				;

	regs->tccrb = ((cfg->mode == PWM_FAST ? 0x1 : 0x0) << TCCRB_WGM2)
				| (0x0 << TCCRB_WGM3)
				| (0x0 << TCCRB_ICNC)
				| (0x0 << TCCRB_ICES)
				| (pres << TCCRB_CS)
				;

	regs->tccrc = (0x0 << TCCRC_FOCA)
				| (0x0 << TCCRC_FOCB)
				;

	*dtd->timsk = 0x0;
	*dtd->tifr = 0x0;

	// NOTE tcnt, ocra, ocrb and icr are not set
	// 		there value doesn't matter on initialisation since the prescaler
	// 		is set to 0, hence the timer is disabled
	// 		later updates to the registers would overwrite the user applied
	// 		settings
	return 0;
}

static int config_get(pwm_cfg_t *cfg, pwm_regs_t *regs){
	cfg->mode = PWM_PHASECORRECT;

	if((bits(regs->tccrb, TCCRB_WGM2, 0x1)) == 0x1)
		cfg->mode = PWM_FAST;

	cfg->base_clock_khz = AVR_IO_CLOCK_HZ / 1000;
	cfg->max = 0x3ff;

	switch(bits(regs->tccrb, TCCRB_CS, 0x7)){
	case 0:	cfg->prescaler = PWM_PRES_0;	break;
	case 1:	cfg->prescaler = PWM_PRES_1;	break;
	case 2:	cfg->prescaler = PWM_PRES_8;	break;
	case 3:	cfg->prescaler = PWM_PRES_64;	break;
	case 4:	cfg->prescaler = PWM_PRES_256;	break;
	case 5:	cfg->prescaler = PWM_PRES_1024;	break;
	default: cfg->prescaler = PWM_PRES_MAX;
	}

	return 0;
}
