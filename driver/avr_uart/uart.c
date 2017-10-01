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


/* macros */
#if (CONFIG_AVR_NUART > 2)
	#error "avr uart driver only supports up to 2 UARTS"
#endif // CONFIG_AVR_NUART


/* local/static prototypes */
static int rx_hdlr(int_num_t num);

static int open(int id, fs_filed_t *fd, f_mode_t mode);
static int read(int id, fs_filed_t *fd, void *buf, size_t n);
static int write(int id, fs_filed_t *fd, void *buf, size_t n);
static int ioctl(int id, fs_filed_t *fd, int request, void *data);

static int config(unsigned int uart, baudrate_t brate, stopb_t stopb, csize_t csize, parity_t parity);
static int putsn(unsigned int uart, char const *s, size_t n);


/* static variables */
// devfs ids and ops
static int dev_ids[CONFIG_AVR_NUART] = { 0 };
static devfs_ops_t dev_ops = {
	.open = open,
	.close = 0x0,
	.read = read,
	.write = write,
	.ioctl = ioctl,
	.fcntl = 0x0
};

// configurations
static uart_t cfg[CONFIG_AVR_NUART] = {{ 0 }};

// receiver buffer
static ringbuf_t rbuf[CONFIG_AVR_NUART] = {{ 0 }};
static uint8_t rx_err[CONFIG_AVR_NUART] = { 0 };

// registers
static int_num_t const rx_int_num[] = { INT_USART0_RX, INT_USART1_RX };
static uint8_t volatile * const ucsra[] = { (uint8_t*)UCSR0A, (uint8_t*)UCSR1A },
						* const ucsrb[] = { (uint8_t*)UCSR0B, (uint8_t*)UCSR1B },
						* const ucsrc[] = { (uint8_t*)UCSR0C, (uint8_t*)UCSR1C },
						* const ubrrl[] = { (uint8_t*)UBRR0L, (uint8_t*)UBRR1L },
						* const ubrrh[] = { (uint8_t*)UBRR0H, (uint8_t*)UBRR1H },
						* const udr[] = { (uint8_t*)UDR0, (uint8_t*)UDR1 };


/* global functions */
char avr_putchar(char c){
	while(!((*(ucsra[0])) & (0x1 << UCSR0A_UDRE)));
	*(udr[0]) = c;

	return c;
}

int avr_puts(char const *s){
	if(s == 0)
		return_errno(E_INVAL);

	for(; *s!=0; s++){
		while(!((*(ucsra[0])) & (0x1 << UCSR0A_UDRE)));
		*(udr[0]) = *s;
	}

	return E_OK;
}


/* local functions */
static int kuart_init(void){
	unsigned int uart;


	for(uart=0; uart<CONFIG_AVR_NUART; uart++)
		config(uart, CONFIG_KERNEL_UART_BAUDRATE, CONFIG_KERNEL_UART_STOPBITS, CS_8, CONFIG_KERNEL_UART_PARITY);

	return errno;
}

platform_init(0, kuart_init);

static int _init(unsigned int uart){
	void *b;
	char name[] = "tty0";


	/* register interrupt handler */
	if(int_hdlr_register(rx_int_num[uart], rx_hdlr) != E_OK)
		goto err_0;

	/* allocate buffers */
	b = kmalloc(CONFIG_AVR_UART_BSIZE);

	if(b == 0x0)
		goto err_1;

	ringbuf_init(rbuf + uart, b, CONFIG_AVR_UART_BSIZE);

	/* register device */
	name[3] = '0' + uart;
	dev_ids[uart] = devfs_dev_register(name, &dev_ops);

	if(dev_ids[uart] < 0)
		goto err_2;

	return E_OK;


err_2:
	kfree(b);

err_1:
	int_hdlr_release(rx_int_num[uart]);

err_0:
	return errno;
}

static int init(void){
	unsigned int i;


	for(i=0; i<CONFIG_AVR_NUART; i++)
		_init(i);

	return errno;
}

driver_init(init);

static int rx_hdlr(int_num_t num){
	unsigned int uart;
	uint8_t c,
			err;


	err = 0;

	/* get uart number */
	for(uart=0; uart<CONFIG_AVR_NUART; uart++){
		if(rx_int_num[uart] == num)
			break;
	}

	/* read data */
	while((*(ucsra[uart]) & (0x1 << UCSR0A_RXC))){
		err |= *(ucsra[uart]) & ((0x1 << UCSR0A_FE) | (0x1 << UCSR0A_DOR) | (0x1 << UCSR0A_UPE));
		c = *(udr[uart]);

		if(err)
			continue;

		if(ringbuf_write(rbuf + uart, &c, 1) != 1)
			err |= (0x1 << UCSR0A_RXC);	// use UCSR0A non-error flag to signal buffer overrun
	}

	/* update error */
	if(err){
		rx_err[uart] |= err;

		cfg[uart].frame_err |= bits(err, UCSR0A_FE, 0x1);
		cfg[uart].data_overrun |= bits(err, UCSR0A_DOR, 0x1);
		cfg[uart].parity_err |= bits(err, UCSR0A_UPE, 0x1);
		cfg[uart].rx_queue_full |= bits(err, UCSR0A_RXC, 0x1);
	}

	return E_OK;
}

static int open(int id, fs_filed_t *fd, f_mode_t mode){
	unsigned int uart;


	/* identify the uart that has been opened */
	for(uart=0; uart<CONFIG_AVR_NUART; uart++){
		if(dev_ids[uart] == id){
			// abuse the file pointer to store the uart index
			fd->fp = uart;

			return E_OK;
		}
	}

	return_errno(E_INVAL);
}

static int read(int id, fs_filed_t *fd, void *buf, size_t n){
	unsigned int uart;


	uart = fd->fp;

	if(rx_err[uart]){
		errno = E_IO;
		rx_err[uart] = 0;

		return 0;
	}

	return ringbuf_read(rbuf + uart, buf, n);
}

static int write(int id, fs_filed_t *fd, void *buf, size_t n){
	unsigned int uart;


	uart = fd->fp;

	if(putsn(uart, buf, n) != E_OK)
		return 0;
	return n;
}

static int ioctl(int id, fs_filed_t *fd, int request, void *data){
	unsigned int uart;
	uart_t *c;


	uart = fd->fp;

	switch(request){
	case IOCTL_CFGRD:
		memcpy(data, cfg + uart, sizeof(uart_t));

		// reset error flags
		cfg[uart].frame_err = 0;
		cfg[uart].data_overrun = 0;
		cfg[uart].parity_err = 0;
		cfg[uart].rx_queue_full = 0;

		rx_err[uart] = 0;

		return E_OK;

	case IOCTL_CFGWR:
		c = (uart_t*)data;
		return config(uart, c->baud, c->stopb, c->csize, c->parity);

	default:
		return E_NOIMP;
	}
}

static int config(unsigned int uart, baudrate_t brate, stopb_t stopb, csize_t csize, parity_t parity){
	static uint8_t const parity_bits[] = { 0b00, 0b11, 0b10 };
	static uint8_t const stopb_bits[] = { 0b0, 0b1 };
	static uint8_t const prr_bits[] = { PRR0_PRUSART0, PRR0_PRUSART1 };
	unsigned int br;


	if(brate > 115200)
		return_errno(E_LIMIT);

	br = (CONFIG_CORE_CLOCK_HZ / (brate * 16));

	if(br == 0)
		return_errno(E_INVAL);

	/* save config */
	cfg[uart].baud = brate;
	cfg[uart].stopb = stopb;
	cfg[uart].csize = csize;
	cfg[uart].parity = parity;

	/* enable uart */
	mreg_w(PRR0, (mreg_r(PRR0) & (-1 ^ (0x1 << prr_bits[uart]))));

	*(ucsra[uart]) = 0x0 << UCSR0A_U2X;
	*(ucsrb[uart]) = (0x1 << UCSR0B_RXEN) |
					 (0x1 << UCSR0B_TXEN) |
					 (0x1 << UCSR0B_RXCIE);

	/* set baudrate */
	*(ubrrh[uart]) = hi8(br);
	*(ubrrl[uart]) = lo8(br);

	/* set csize, parity, stop bits */
	*(ucsrc[uart]) = (csize << UCSR0C_UCSZ0) |
					 (parity_bits[parity] << UCSR0C_UPM0) |
					 (stopb_bits[stopb] << UCSR0C_USBS);

	return_errno(E_OK);
}

static int putsn(unsigned int uart, char const *s, size_t n){
	if(s == 0)
		return_errno(E_INVAL);

	for(; n>0; n--, s++){
		while(!(*(ucsra[uart]) & (0x1 << UCSR0A_UDRE)));
		*(udr[uart]) = *s;
	}

	return E_OK;
}
