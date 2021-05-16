/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/stat.h>
#include <sys/uart.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <termios.h>
#include <pthread.h>
#include <hardware/hardware.h>


/* types */
typedef struct{
	char path[64];
	int fd;
	int int_num;
} uart_t;


/* local/static prototypes */
static int add(char const *path, int int_num);
static int configure(int fd, uart_cfg_t *cfg);

static speed_t baudrate(uart_baudrate_t br);

static void sig_hdlr(int sig);


/* static variables */
static pthread_t uart_tid = -1;
static pthread_mutex_t uart_mtx = PTHREAD_MUTEX_INITIALIZER;

static uart_t *uarts = 0x0;
static size_t nuart = 0;


/* global functions */
int uart_init(void){
	int r;
	sigset_t sig_lst;


	uart_tid = pthread_self();

	(void)signal(CONFIG_TEST_INT_UART_SIG, sig_hdlr);

	r = 0;

	r |= sigemptyset(&sig_lst);
	r |= sigaddset(&sig_lst, CONFIG_TEST_INT_UART_SIG);

	r |= pthread_sigmask(SIG_UNBLOCK, &sig_lst, 0x0);

	return r;
}

void uart_cleanup(void){
	size_t i;


	pthread_mutex_lock(&uart_mtx);

	for(i=0; i<nuart; i++)
		close(uarts[i].fd);

	free(uarts);
	nuart = 0;

	pthread_mutex_unlock(&uart_mtx);
}

void uart_poll(void){
	size_t i,
		   nfd = nuart;
	struct pollfd fds[nfd];


	if(nfd == 0){
		sleep(100);
		return;
	}

	pthread_mutex_lock(&uart_mtx);

	for(i=0; i<nfd; i++){
		fds[i].fd = (uarts[i].int_num != 0 )? uarts[i].fd : -1;
		fds[i].events = POLLIN;
	}

	pthread_mutex_unlock(&uart_mtx);

	if(poll(fds, nfd, -1) <= 0)
		return;

	for(i=0; i<nfd; i++){
		if(fds[i].revents & POLLIN)
			hw_int_request(uarts[i].int_num, 0x0, PRIV_HARDWARE, 0);
	}
}

int uart_configure(char const *path, int int_num, uart_cfg_t *cfg){
	int r;
	int fd;


	fd = add(path, int_num);

	if(fd < 0)
		return -1;

	r = 0;

	r |= configure(fd, cfg);
	r |= pthread_kill(uart_tid, CONFIG_TEST_INT_UART_SIG);

	return r;
}


/* local functions */
static int add(char const *path, int int_num){
	size_t i;
	int fd;


	for(i=0; i<nuart; i++){
		if(strcmp(uarts[i].path, path) == 0)
			return uarts[i].fd;
	}

	fd = open(path, O_RDWR);

	if(fd < 0)
		goto err_0;

	pthread_mutex_lock(&uart_mtx);

	uarts = realloc(uarts, sizeof(uart_t) * (nuart + 1));

	if(uarts == 0x0)
		goto err_1;

	strcpy(uarts[nuart].path, path);
	uarts[nuart].fd = fd;
	uarts[nuart].int_num = int_num;

	nuart++;

	pthread_mutex_unlock(&uart_mtx);

	return fd;


err_1:
	pthread_mutex_unlock(&uart_mtx);
	close(fd);

err_0:
	return -1;
}

static int configure(int fd, uart_cfg_t *cfg){
	struct termios attr;
	int cs[] = { CS5, CS6, CS7, CS8 };
	int parity[] = { 0, PARENB, PARENB | PARODD };


	if(cfsetspeed(&attr, baudrate(cfg->baudrate)) != 0)
		return -1;

	if(tcgetattr(fd, &attr) != 0)
		return -1;

	attr.c_oflag = ONLCR | OPOST;
	attr.c_lflag = IEXTEN | ECHOCTL | ECHOK | ECHOE | ISIG
				 | ((cfg->flags & TERM_FLAG_ECHO) ? ECHO : 0)
				 ;

	attr.c_cflag = CBAUDEX | CLOCAL | HUPCL | CREAD
				 | (cs[cfg->csize])
				 | (parity[cfg->parity])
				 | ((cfg->stopb == UART_STOPB2) ? CSTOPB : 0)
				 ;

	if(tcsetattr(fd, TCSANOW, &attr) != 0)
		return -1;

	return 0;
}

static speed_t baudrate(uart_baudrate_t br){
	switch(br){
	case UART_BR_2400:		return B2400; break;
	case UART_BR_4800:		return B4800; break;
	case UART_BR_9600:		return B9600; break;
	case UART_BR_19200:		return B19200; break;
	case UART_BR_38400:		return B38400; break;
	case UART_BR_57600:		return B57600; break;
	case UART_BR_115200:	return B115200; break;
	case UART_BR_230400:	return B230400; break;
	case UART_BR_500000:	return B500000; break;
	case UART_BR_1000000:	return B1000000; break;
	case UART_BR_0:			// fall through
	case UART_BR_14400:		// fall through
	case UART_BR_28800:		// fall through
	case UART_BR_76800:		// fall through
	case UART_BR_250000:	// fall through
	default:				return B0; break;
	}
}

static void sig_hdlr(int sig){
	// only used to allow interrupting poll()
	// and update the uart list
}
