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

#define TCCRB_FOCA	7
#define TCCRB_FOCB	6
#define TCCRB_WGM2	3
#define TCCRB_CS	0


/* types */
typedef struct{
	// device registers
	struct{
		uint8_t volatile tccra,
						 tccrb,
						 tcnt,
						 ocra,
						 ocrb;
	} *dev;

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
static size_t read(unsigned int *buf, size_t n, uint8_t volatile *reg);
static size_t write(unsigned int *buf, size_t n, uint8_t volatile *reg);
static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);
static int config_set(pwm_cfg_t *cfg, dt_data_t *regs);
static int config_get(pwm_cfg_t *cfg, dt_data_t *regs);


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	size_t name_len = strlen(name);
	char cname[name_len + 2];
	devfs_dev_t *dev;
	devfs_ops_t ops;
	pwm_cfg_t cfg;


	/* configure timer0 */
	cfg.mode = PWM_FAST;
	cfg.base_clock_khz = 20000;
	cfg.prescaler = PWM_PRES_0;

	if(config_set(&cfg, dt_data) != 0)
		return -errno;

	/* register device */
	strcpy(cname, name);
	cname[name_len + 1] = 0;

	ops.open = 0x0;
	ops.close = 0x0;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;

	cname[name_len] = 'a';
	ops.read = read_a;
	ops.write = write_a;

	if((dev = devfs_dev_register(cname, &ops, 0x0, dt_data)) == 0x0)
		goto err_0;

	cname[name_len] = 'b';
	ops.read = read_b;
	ops.write = write_b;

	if(devfs_dev_register(cname, &ops, 0x0, dt_data) == 0x0)
		goto err_1;

	return E_OK;

err_1:
	devfs_dev_release(dev);

err_0:
	return -errno;
}

driver_device("avr,pwm8", probe);

static size_t read_a(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return read(buf, n, &((dt_data_t*)dev->data)->dev->ocra);
}

static size_t write_a(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return write(buf, n, &((dt_data_t*)dev->data)->dev->ocra);
}

static size_t read_b(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return read(buf, n, &((dt_data_t*)dev->data)->dev->ocrb);
}

static size_t write_b(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	return write(buf, n, &((dt_data_t*)dev->data)->dev->ocrb);
}

static size_t read(unsigned int *buf, size_t n, uint8_t volatile *reg){
	if(n != sizeof(*buf)){
		errno = E_INVAL;
		return 0;
	}

	*buf = *reg;
	return sizeof(*buf);
}

static size_t write(unsigned int *buf, size_t n, uint8_t volatile *reg){
	if(n != sizeof(*buf)){
		errno = E_INVAL;
		return 0;
	}

	*reg = (*buf) & 0xff;
	return sizeof(*buf);
}

static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	switch(request){
	case IOCTL_CFGRD:	return config_get(data, dev->data);
	case IOCTL_CFGWR:	return config_set(data, dev->data);
	default:			return_errno(E_NOIMP);
	}
}

static int config_set(pwm_cfg_t *cfg, dt_data_t *regs){
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
	*regs->ddr |= regs->ddr_en;

	/* enable timer */
	*regs->prr &= ~regs->prr_en;

	/* set mode */
	regs->dev->tccra = (0x2 << TCCRA_COMA)
					 | (0x2 << TCCRA_COMB)
					 | ((cfg->mode == PWM_FAST ? 0x3 : 0x1) << TCCRA_WGM0)
					 ;

	regs->dev->tccrb = (0x0 << TCCRB_WGM2)
					 | (0x0 << TCCRB_FOCA)
					 | (0x0 << TCCRB_FOCB)
					 | (pres << TCCRB_CS)
					 ;

	*regs->timsk = 0x0;
	*regs->tifr = 0x0;

	// NOTE tcnt, ocra and ocrb are not set
	// 		there value doesn't matter on initialisation since the prescaler
	// 		is set to 0, hence the timer is disabled
	// 		later updates to the registers would overwrite the user applied
	// 		settings

	return E_OK;
}

static int config_get(pwm_cfg_t *cfg, dt_data_t *regs){
	cfg->mode = PWM_PHASECORRECT;

	if((bits(regs->dev->tccra, TCCRA_WGM0, 0x3)) == 0x3)
		cfg->mode = PWM_FAST;

	cfg->base_clock_khz = AVR_IO_CLOCK_HZ / 1000;
	cfg->max = 0xff;

	switch(bits(regs->dev->tccrb, TCCRB_CS, 0x7)){
	case 0:	cfg->prescaler = PWM_PRES_0;	break;
	case 1:	cfg->prescaler = PWM_PRES_1;	break;
	case 2:	cfg->prescaler = PWM_PRES_8;	break;
	case 3:	cfg->prescaler = PWM_PRES_64;	break;
	case 4:	cfg->prescaler = PWM_PRES_256;	break;
	case 5:	cfg->prescaler = PWM_PRES_1024;	break;
	default: cfg->prescaler = PWM_PRES_MAX;
	}

	return E_OK;
}