/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/interrupt.h>
#include <kernel/driver.h>
#include <kernel/devfs.h>
#include <kernel/kprintf.h>
#include <kernel/inttask.h>
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
	uint8_t volatile eecr,
					 eedr;
	uint16_t volatile eear;
} eeprom_regs_t;

typedef struct{
	// device registers
	eeprom_regs_t *regs;

	// memory properties
	void *base;
	size_t size;

	// interrupt
	uint8_t const int_num;
} dt_data_t;

typedef struct{
	uint8_t *buf;
	size_t len;
	size_t offset;
} write_data_t;

typedef struct{
	dt_data_t *dtd;

	itask_queue_t write_queue;
} dev_data_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int fcntl(devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data);

static size_t write_noint(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n);
static size_t write_int(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n);

static void write_byte(uint8_t b, size_t offset, dt_data_t *dtd);
static char read_byte(size_t offset, dt_data_t *dtd);

static void write_hdlr(int_num_t num, void *eeprom);
static int write_complete(void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *dtd;
	devfs_dev_t *dev;
	devfs_ops_t ops;
	dev_data_t *eeprom;


	dtd = (dt_data_t*)dt_data;

	/* allocate eeprom */
	eeprom = kmalloc(sizeof(dev_data_t));

	if(eeprom == 0x0)
		goto err_0;

	eeprom->dtd = dtd;
	itask_queue_init(&eeprom->write_queue);

	/* register interrupt */
	if(dtd->int_num && int_register(dtd->int_num, write_hdlr, eeprom) != 0)
		goto err_1;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = 0x0;
	ops.fcntl = fcntl;
	ops.mmap = 0x0;

	dev = devfs_dev_register(name, &ops, eeprom);

	if(dev == 0x0)
		goto err_2;

	if(dtd->int_num)
		dev->node->timeout_us = 0;

	/* configure hardware */
	dtd->regs->eecr = 0x0;

	return 0x0;


err_2:
	int_release(dtd->int_num);

err_1:
	kfree(eeprom);

err_0:
	return 0x0;
}

driver_probe("avr,eeprom", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *_buf, size_t n){
	size_t i;
	char *buf;
	dt_data_t *dtd;


	buf = (char*)_buf;
	dtd = ((dev_data_t*)dev->data)->dtd;

	if(fd->fp + n >= dtd->size){
		errno = E_LIMIT;
		return 0;
	}

	for(i=0; i<n; i++){
		buf[i] = read_byte(fd->fp, dtd);
		fd->fp++;
	}

	return i;
}

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dt_data_t *dtd;


	dtd = ((dev_data_t*)dev->data)->dtd;

	if(fd->fp + n >= dtd->size){
		errno = E_LIMIT;
		return 0;
	}

	if(n <= 1 || dtd->int_num == 0)
		return write_noint(dev->data, fd, buf, n);

	return write_int(dev->data, fd, buf, n);
}

static int fcntl(struct devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *_data){
	size_t whence;
	seek_t *data;
	dt_data_t *dtd;


	data = (seek_t*)_data;
	dtd = ((dev_data_t*)dev->data)->dtd;

	switch(cmd){
	case F_SEEK:
		if(data->whence == SEEK_SET)		whence = 0;
		else if(data->whence == SEEK_CUR)	whence = fd->fp;
		else if(data->whence == SEEK_END)	whence = dtd->size - 1;
		else								return_errno(E_NOIMP);

		if(whence + data->offset >= dtd->size)
			return_errno(E_LIMIT);

		fd->fp = whence + data->offset;
		return 0;

	case F_TELL:
		data->pos = fd->fp;
		return 0;

	default:
		return E_NOSUP;
	};
}

static size_t write_noint(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n){
	size_t i;


	for(i=0; i<n; i++){
		write_byte(buf[i], fd->fp, eeprom->dtd);
		fd->fp++;
	}

	return n;
}

static size_t write_int(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n){
	write_data_t data;


	data.buf = buf;
	data.len = n;
	data.offset = fd->fp;

	(void)itask_issue(&eeprom->write_queue, &data, eeprom->dtd->int_num);

	return n - data.len;
}

static void write_byte(uint8_t b, size_t offset, dt_data_t *dtd){
	eeprom_regs_t *regs;
	int_type_t imask;


	DEBUG("write %#4.4x: %c (%#hhx)\n", dtd->base + offset, b, b);

	regs = dtd->regs;

	while(regs->eecr & (0x1 << EECR_EEPE));

	imask = int_enable(INT_NONE);	// the write sequence must not be interrupted

	regs->eear = (uint16_t)dtd->base + offset;
	regs->eedr = b;

	asm volatile(
		"ori	%[eecr], %[prot_mask]\n"
		"st		%a[eecr_p], %[eecr]\n"
		"or		%[eecr], %[write_mask]\n"
		"st		%a[eecr_p], %[eecr]\n"
		:
		: [eecr_p] "e" (&regs->eecr),
		  [eecr] "r" (regs->eecr),
		  [prot_mask] "i" (0x1 << EECR_EEMPE),
		  [write_mask] "r" (0x1 << EECR_EEPE | ((dtd->int_num ? 0x1 : 0x0) << EECR_EERIE))
	);

	int_enable(imask);
}

static char read_byte(size_t offset, dt_data_t *dtd){
	char c;
	int_type_t imask;
	eeprom_regs_t *regs;


	regs = dtd->regs;

	while(regs->eecr & (0x1 << EECR_EEPE));

	imask = int_enable(INT_NONE);

	regs->eear = (uint16_t)(dtd->base + offset);
	regs->eecr |= (0x1 << EECR_EERE);
	c = regs->eedr;

	int_enable(imask);

	DEBUG("read %#4.4x: %c (%#hhx)\n", dtd->base + fd->fp, buf[i], buf[i]);

	return c;
}

static void write_hdlr(int_num_t num, void *_eeprom){
	dev_data_t *eeprom;
	write_data_t *data;


	eeprom = (dev_data_t*)_eeprom;

	/* disable eeprom interrupt */
	// NOTE this is required since an interrupt is triggered
	// 		as long as EECR_EEPE is zero, i.e. always except
	// 		during ongoing write operations
	eeprom->dtd->regs->eecr &= ~(0x1 << EECR_EERIE);
	data = itask_query_data(&eeprom->write_queue, write_complete);

	if(data == 0x0)
		return;

	/* write data */
	write_byte(*data->buf, data->offset, eeprom->dtd);

	data->buf++;
	data->offset++;
	data->len--;
}

static int write_complete(void *data){
	return (((write_data_t*)data)->len == 0) ? 0 : -1;
}
