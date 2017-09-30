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


/* local/static prototypes */
static int read(int id, fs_filed_t *fd, void *buf, size_t n);
static int write(int id, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(int id, fs_filed_t *fd, int request, void *data);

static int config(baudrate_t brate, stopb_t stopb, csize_t csize, parity_t parity);
static int putsn(char const *s, size_t n);


/* static variables */
static int dev_id;
static uart_config_t cfg;


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
	devfs_ops_t ops;


/* register device */
	ops.open = 0x0;
	ops.close = 0x0;
	ops.read = read;
	ops.write = write;
	ops.ioctl = ioctl;
	ops.fcntl = 0x0;

	dev_id = devfs_dev_register("tty0", &ops);

	if(dev_id < 0)
		return errno;

	return E_OK;
}

driver_init(init);

static int read(int id, fs_filed_t *fd, void *buf, size_t n){
	size_t i;


	for(i=0; i<n; i++){
		while(!(mreg_r(UCSR0A) & (0x1 << UCSR0A_RXC)));
		((char*)buf)[i] = mreg_r(UDR0);
	}

	return n;
}

static int write(int id, fs_filed_t *fd, void *buf, size_t n){
	if(putsn(buf, n) != E_OK)
		return 0;
	return n;
}

static int ioctl(int id, fs_filed_t *fd, int request, void *data){
	uart_config_t *c;


	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, &cfg, sizeof(uart_config_t));
		return E_OK;

	case IOCTL_CFGWR:
		c = (uart_config_t*)data;
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
		(0x0 << UCSR0B_RXCIE)
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
