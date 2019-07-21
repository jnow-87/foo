/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <sys/types.h>
#include <sys/errno.h>


/* macros */
// register bits
#define EECR_EEPM1		5
#define EECR_EEPM0		4
#define EECR_EERIE		3
#define EECR_EEMPE		2
#define EECR_EEPE		1
#define EECR_EERE		0


/* types */
typedef struct{
	struct{
		uint8_t volatile eecr,
						 eedr;
		uint16_t volatile eear;
	} *dev;

	void *base;
	size_t size;
} dt_data_t;


/* local/static prototypes */
static int probe(char const *name, void *dt_data, void *dt_itf);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int fcntl(struct devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data);


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *regs;
	devfs_ops_t ops;


	/* configure hardware */
	regs = (dt_data_t*)dt_data;

	regs->dev->eecr = (0x0 << EECR_EEPM0)
					| (0x0 << EECR_EERIE)
					;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = 0x0;
	ops.fcntl = fcntl;

	(void)devfs_dev_register(name, &ops, 0, dt_data);

	return -errno;
}

device_probe("avr,eeprom", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *_buf, size_t n){
	size_t i;
	char *buf;
	dt_data_t *regs;
	int_type_t imask;


	buf = (char*)_buf;
	regs = dev->data;

	if(fd->fp + n >= regs->size)
		return_errno(E_LIMIT);

	imask = int_enable(INT_NONE);

	for(i=0; i<n; i++){
		while(regs->dev->eecr & (0x1 << EECR_EEPE));

		regs->dev->eear = (uint16_t)(regs->base + fd->fp);
		regs->dev->eecr |= (0x1 << EECR_EERE);
		buf[i] = regs->dev->eedr;

		DEBUG("read %#4.4x: %c (%#hhx)\n", regs->base + fd->fp, buf[i], buf[i]);
		fd->fp++;
	}

	int_enable(imask);

	return i;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *_buf, size_t n){
	size_t i;
	uint8_t *buf;
	dt_data_t *regs;
	int_type_t imask;


	buf = (uint8_t*)_buf;
	regs = dev->data;

	if(fd->fp + n >= regs->size)
		return_errno(E_LIMIT);

	imask = int_enable(INT_NONE);

	for(i=0; i<n; i++){
		while(regs->dev->eecr & (0x1 << EECR_EEPE));

		regs->dev->eear = (uint16_t)regs->base + fd->fp;
		regs->dev->eedr = buf[i];

		asm volatile(
			"ori	%[eecr], %[pme_mask]\n"
			"st		%a[eecr_p], %[eecr]\n"
			"ori	%[eecr], %[pe_mask]\n"
			"st		%a[eecr_p], %[eecr]\n"
			:
			: [eecr_p] "e" (&regs->dev->eecr),
			  [eecr] "r" (regs->dev->eecr),
			  [pme_mask] "i" (0x1 << EECR_EEMPE),
			  [pe_mask] "i" (0x1 << EECR_EEPE)
		);

		DEBUG("write %#4.4x: %c (%#hhx)\n", regs->base + fd->fp, buf[i], buf[i]);
		fd->fp++;
	}

	int_enable(imask);

	return i;
}

static int fcntl(struct devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *_data){
	size_t whence;
	seek_t *data;
	dt_data_t *regs;


	data = (seek_t*)_data;
	regs = (dt_data_t*)dev->data;

	switch(cmd){
	case F_SEEK:
		if(data->whence == SEEK_SET)		whence = 0;
		else if(data->whence == SEEK_CUR)	whence = fd->fp;
		else if(data->whence == SEEK_END)	whence = regs->size - 1;
		else								return_errno(E_NOIMP);

		if(whence + data->offset >= regs->size)
			return_errno(E_LIMIT);

		fd->fp = whence + data->offset;
		return E_OK;

	case F_TELL:
		data->pos = fd->fp;
		return E_OK;

	default:
		return E_NOSUP;
	};
}
