#include <config/config.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/init.h>
#include <kernel/devfs.h>
#include <kernel/kmem.h>
#include <kernel/kprintf.h>
#include <sys/ringbuf.h>
#include <sys/uart.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/string.h>
#include <sys/register.h>


/* local/static prototypes */
static int rx_hdlr(int_num_t num);

static int read(int id, fs_filed_t *fd, void *buf, size_t n);
static int write(int id, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(int id, fs_filed_t *fd, int request, void *data);

static int config(baudrate_t brate, stopb_t stopb, csize_t csize, parity_t parity);
static int putsn(char const *s, size_t n);


/* static variables */
static int dev_id;
static ringbuf_t rbuf;
static uart_t cfg;
static uint8_t rx_err;


/* global functions */
char avr_putchar(char c){
	while(!(mreg_r(UCSR0A) & (0x1 << UCSR0A_UDRE)));
	mreg_w(UDR0, c);

	return c;
}

int avr_puts(char const *s){
	if(s == 0)
		return_errno(E_INVAL);

	for(; *s!=0; s++){
		while(!(mreg_r(UCSR0A) & (0x1 << UCSR0A_UDRE)));
		mreg_w(UDR0, *s);
	}

	return E_OK;
}


/* local functions */
static int kuart_init(void){
	return config(CONFIG_KERNEL_UART_BAUDRATE, CONFIG_KERNEL_UART_STOPBITS, CS_8, CONFIG_KERNEL_UART_PARITY);
}

platform_init(0, kuart_init);

static int init(void){
	void *b;
	devfs_ops_t ops;


	/* register interrupt handler */
	if(int_hdlr_register(INT_USART0_RX, rx_hdlr) != E_OK)
		goto err_0;

	/* allocate buffers */
	b = kmalloc(CONFIG_AVR_UART_BSIZE);

	if(b == 0x0)
		goto err_1;

	ringbuf_init(&rbuf, b, CONFIG_AVR_UART_BSIZE);

	/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;

	dev_id = devfs_dev_register("tty0", &ops);

	if(dev_id < 0)
		goto err_2;

	return E_OK;


err_2:
	kfree(b);

err_1:
	int_hdlr_release(INT_USART0_RX);

err_0:
	return errno;
}

driver_init(init);

static int rx_hdlr(int_num_t num){
	uint8_t c,
			err;


	err = 0;

	while((mreg_r(UCSR0A) & (0x1 << UCSR0A_RXC))){
		err |= mreg_r(UCSR0A) & ((0x1 << UCSR0A_FE) | (0x1 << UCSR0A_DOR) | (0x1 << UCSR0A_UPE));
		c = mreg_r(UDR0);

		if(err)
			continue;

		if(ringbuf_write(&rbuf, &c, 1) != 1)
			err |= (0x1 << UCSR0A_RXC);	// use UCSR0A non-error flag to signal buffer overrun
	}

	if(err){
		rx_err |= err;

		cfg.frame_err |= bits(err, UCSR0A_FE, 0x1);
		cfg.data_overrun |= bits(err, UCSR0A_DOR, 0x1);
		cfg.parity_err |= bits(err, UCSR0A_UPE, 0x1);
		cfg.rx_queue_full |= bits(err, UCSR0A_RXC, 0x1);
	}

	return E_OK;
}

static int read(int id, fs_filed_t *fd, void *buf, size_t n){
	if(rx_err){
		errno = E_IO;
		rx_err = 0;

		return 0;
	}

	return ringbuf_read(&rbuf, buf, n);
}

static int write(int id, fs_filed_t *fd, void *buf, size_t n){
	if(putsn(buf, n) != E_OK)
		return 0;
	return n;
}

static int ioctl(int id, fs_filed_t *fd, int request, void *data){
	uart_t *c;


	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, &cfg, sizeof(uart_t));

		// reset error flags
		cfg.frame_err = 0;
		cfg.data_overrun = 0;
		cfg.parity_err = 0;
		cfg.rx_queue_full = 0;

		rx_err = 0;

		return E_OK;

	case IOCTL_CFGWR:
		c = (uart_t*)data;
		return config(c->baud, c->stopb, c->csize, c->parity);

	default:
		return E_NOIMP;
	}
}

static int config(baudrate_t brate, stopb_t stopb, csize_t csize, parity_t parity){
	static uint8_t const parity_bits[] = { 0b00, 0b11, 0b10 };
	static uint8_t const stopb_bits[] = { 0b0, 0b1 };
	unsigned int br;


	if(brate > 115200)
		return_errno(E_LIMIT);

	br = (CONFIG_CORE_CLOCK_HZ / (brate * 16));

	if(br == 0)
		return_errno(E_INVAL);

	/* save config */
	cfg.baud = brate;
	cfg.stopb = stopb;
	cfg.csize = csize;
	cfg.parity = parity;

	/* enable uart */
	mreg_w(PRR0, (mreg_r(PRR0) & (-1 ^ (0x1 << PRR0_PRUSART0))));
	mreg_w(UCSR0A, 0x0 << UCSR0A_U2X);
	mreg_w(UCSR0B,
		(0x1 << UCSR0B_RXEN) |
		(0x1 << UCSR0B_TXEN) |
		(0x1 << UCSR0B_RXCIE)
	);

	/* set baudrate */
	mreg_w(UBRR0H, (br >> 8));
	mreg_w(UBRR0L, (br & 0xff));

	/* set csize, parity, stop bits */
	mreg_w(UCSR0C,
		(csize << UCSR0C_UCSZ0) |
		(parity_bits[parity] << UCSR0C_UPM0) |
		(stopb_bits[stopb] << UCSR0C_USBS)
	);

	return_errno(E_OK);
}

static int putsn(char const *s, size_t n){
	if(s == 0)
		return_errno(E_INVAL);

	for(; n>0; n--, s++){
		while(!(mreg_r(UCSR0A) & (0x1 << UCSR0A_UDRE)));
		mreg_w(UDR0, *s);
	}

	return E_OK;
}
