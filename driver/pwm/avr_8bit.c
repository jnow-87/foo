/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/avr/atmega.h>
#include <kernel/init.h>
#include <kernel/devfs.h>
#include <sys/types.h>


/* types */
typedef struct{
	uint8_t *tccra,
			*tccrb,
			*timsk,
			*tifr;

	uint8_t *tcnt,
			*ocra,
			*ocrb;
} pwm8_reg_map_t;


/* local/static prototypes */
static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);
static int config(pwm8_reg_map_t *reg_map);


/* local functions */
static int init(void){
	devfs_ops_t ops;
	pwm8_reg_map_t map;


	/* configure timer0 */
	map.tccra = (uint8_t*)TCCR0A;
	map.tccrb = (uint8_t*)TCCR0B;
	map.timsk = (uint8_t*)TIMSK0;
	map.tifr = (uint8_t*)TIFR0;
	map.tcnt = (uint8_t*)TCNT0;
	map.ocra = (uint8_t*)OCR0A;
	map.ocrb = (uint8_t*)OCR0B;

	if(config(&map) != 0)
		return -errno;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;

	if(devfs_dev_register("pwm", &ops, 0x0) == 0x0)
		return -errno;

	return 0;
}

driver_init(init);

static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	// TODO
	return 0;
}

static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	// TODO
	return 0;
}

static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	// TODO
	return 0;
}

static int config(pwm8_reg_map_t *reg_map){
	// TODO
	// enable timer in PRR
	// set clock select
	// set mode
	return 0;
}
