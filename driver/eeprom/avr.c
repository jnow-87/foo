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
#include <kernel/sigqueue.h>
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

	uint8_t const int_num;
} dt_data_t;

typedef struct{
	uint8_t *buf;
	size_t len;
	size_t offset;
} write_data_t;

typedef struct{
	dt_data_t *regs;

	sigq_queue_t write_queue;
} dev_data_t;


/* local/static prototypes */
static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n);
static int fcntl(struct devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *data);

static size_t write_noint(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n);
static size_t write_int(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n);
static void write_byte(uint8_t b, size_t offset, dt_data_t *regs);
static void write_hdlr(int_num_t num, void *eeprom);


/* local functions */
static int probe(char const *name, void *dt_data, void *dt_itf){
	dt_data_t *regs;
	devfs_ops_t ops;
	dev_data_t *eeprom;


	regs = (dt_data_t*)dt_data;

	/* allocate eeprom */
	eeprom = kmalloc(sizeof(dev_data_t));

	if(eeprom == 0x0)
		goto err_0;

	eeprom->regs = regs;
	sigq_queue_init(&eeprom->write_queue);

	/* register interrupt */
	if(regs->int_num && int_register(regs->int_num, write_hdlr, eeprom) != 0)
		goto err_1;

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = 0x0;
	ops.fcntl = fcntl;

	if(devfs_dev_register(name, &ops, 0, eeprom) == 0x0)
		goto err_2;

	/* configure hardware */
	regs->dev->eecr = 0x0;

	return E_OK;


err_2:
	int_release(regs->int_num);

err_1:
	kfree(eeprom);

err_0:
	return -errno;
}

device_probe("avr,eeprom", probe);

static size_t read(devfs_dev_t *dev, fs_filed_t *fd, void *_buf, size_t n){
	size_t i;
	char *buf;
	dt_data_t *regs;
	int_type_t imask;


	buf = (char*)_buf;
	regs = ((dev_data_t*)dev->data)->regs;

	if(fd->fp + n >= regs->size){
		errno = E_LIMIT;
		return 0;
	}

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

static size_t write(devfs_dev_t *dev, fs_filed_t *fd, void *buf, size_t n){
	dt_data_t *regs;


	regs = ((dev_data_t*)dev->data)->regs;

	if(fd->fp + n >= regs->size){
		errno = E_LIMIT;
		return 0;
	}

	if(n <= 1 || regs->int_num == 0)
		return write_noint(dev->data, fd, buf, n);
	return write_int(dev->data, fd, buf, n);
}

static int fcntl(struct devfs_dev_t *dev, fs_filed_t *fd, int cmd, void *_data){
	size_t whence;
	seek_t *data;
	dt_data_t *regs;


	data = (seek_t*)_data;
	regs = ((dev_data_t*)dev->data)->regs;

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

static size_t write_noint(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n){
	size_t i;


	for(i=0; i<n; i++){
		write_byte(buf[i], fd->fp, eeprom->regs);
		fd->fp++;
	}

	return n;
}

static size_t write_int(dev_data_t *eeprom, fs_filed_t *fd, uint8_t *buf, size_t n){
	write_data_t data;
	sigq_t e;


	data.buf = buf;
	data.len = n;
	data.offset = fd->fp;

	sigq_init(&e, &data);

	if(sigq_enqueue(&eeprom->write_queue, &e))
		write_hdlr(eeprom->regs->int_num, eeprom);

	sigq_wait(&e);

	return n;
}

static void write_byte(uint8_t b, size_t offset, dt_data_t *regs){
	int_type_t imask;


	DEBUG("write %#4.4x: %c (%#hhx)\n", regs->base + offset, b, b);

	imask = int_enable(INT_NONE);	// ensure the write sequence is never interrupted

	while(regs->dev->eecr & (0x1 << EECR_EEPE));

	regs->dev->eear = (uint16_t)regs->base + offset;
	regs->dev->eedr = b;

	asm volatile(
		"ori	%[eecr], %[prot_mask]\n"
		"st		%a[eecr_p], %[eecr]\n"
		"or		%[eecr], %[write_mask]\n"
		"st		%a[eecr_p], %[eecr]\n"
		:
		: [eecr_p] "e" (&regs->dev->eecr),
		  [eecr] "r" (regs->dev->eecr),
		  [prot_mask] "i" (0x1 << EECR_EEMPE),
		  [write_mask] "r" (0x1 << EECR_EEPE | ((regs->int_num ? 0x1 : 0x0) << EECR_EERIE))
	);

	int_enable(imask);
}

static void write_hdlr(int_num_t num, void *_eeprom){
	static sigq_t *e = 0x0;
	dev_data_t *eeprom;
	write_data_t *data;


	eeprom = (dev_data_t*)_eeprom;

	/* disable interrupt */
	// NOTE ithis is required since an interrupt is triggered
	// 		as long as EECR_EEPE is zero, i.e. always except
	// 		during ongoing write operations
	eeprom->regs->dev->eecr &= ~(0x1 << EECR_EERIE);

	/* get next write queue element */
	if(e == 0x0)
		e = sigq_first(&eeprom->write_queue);

	/* write data */
	if(e){
		data = e->data;

		write_byte(*data->buf, data->offset, eeprom->regs);
		data->buf++;
		data->offset++;
		data->len--;

		// signal write complete
		if(data->len == 0){
			sigq_dequeue(&eeprom->write_queue, e);
			e = 0x0;
		}
	}
}
