#ifndef DRIVER_UART_H
#define DRIVER_UART_H


#include <config/config.h>
#include <sys/ringbuf.h>
#include <sys/uart.h>


/* types */
typedef struct{
	int (*config)(unsigned int uart, uart_t *cfg);
	int (*puts)(unsigned int uart, char const *s, size_t n);
} uart_cbs_t;


/* external variables */
extern uart_t uart_cfg[CONFIG_NUM_UART];
extern ringbuf_t uart_rx_buf[CONFIG_NUM_UART];
extern unsigned int uart_rx_err[CONFIG_NUM_UART];


#endif // DRIVER_UART_H
