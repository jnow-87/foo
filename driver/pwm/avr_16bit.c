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
#include <sys/errno.h>
#include <sys/types.h>


/* types */
typedef struct{
	uint8_t *tccra,
			*tccrb,
			*tccrc,
			*timsk,
			*tifr;

	uint16_t *tcnt,
			 *ocra,
			 *ocrb,
			 *ocrc,
			 *icr;
} pwm16_reg_map_t;


/* local/static prototypes */
static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data);
static int config(pwm16_reg_map_t *reg_map);


/* local functions */
static int init(void){
	// TODO
	return_errno(E_NOIMP);
}

driver_init(init);

static int read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	// TODO
	return_errno(E_NOIMP);
}

static int write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	// TODO
	return_errno(E_NOIMP);
}

static int ioctl(struct devfs_dev_t *dev, fs_filed_t *fd, int request, void *data){
	// TODO
	return_errno(E_NOIMP);
}

static int config(pwm16_reg_map_t *reg_map){
	// TODO
	return_errno(E_NOIMP);
}
