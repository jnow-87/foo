#ifndef DRIVER_UART_H
#define DRIVER_UART_H


#include <config/config.h>
#include <kernel/signal.h>
#include <sys/ringbuf.h>
#include <sys/uart.h>


/* types */
typedef struct{
	int (*config)(unsigned int uart, uart_t *cfg);
	int (*puts)(unsigned int uart, char const *s, size_t n);
} uart_cbs_t;

typedef struct{
	uart_t cfg;

	ksignal_t *rx_rdy;
	ringbuf_t rx_buf;
	unsigned int rx_err;
} kuart_t;


/* external variables */
extern kuart_t uarts[CONFIG_UART_CNT];


#endif // DRIVER_UART_H
