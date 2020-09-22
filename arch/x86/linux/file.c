/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <arch/x86/opts.h>
#include <sys/stdarg.h>
#include <sys/types.h>
#include <sys/stream.h>


/* local/static prototypes */
static char lnx_putc(char c, struct FILE *stream);


/* global functions */
long int lnx_open(char const *path, int flags, int mode){
	return lnx_syscall(LNX_SYS_OPEN,
		(unsigned long int[6]){
			(unsigned long int)path,
			flags,
			mode,
			0,
			0,
			0
		},
		1
	);
}

void lnx_close(int fd){
	long int r;


	r = lnx_syscall(LNX_SYS_CLOSE, (unsigned long int[6]){ fd, 0, 0, 0, 0, 0 }, 1);

	if(r != 0)
		LNX_SYSCALL_ERROR_EXIT("%d != %d", r, 0);
}

void lnx_read(int fd, void *buf, size_t n){
	size_t i;
	ssize_t r;


	for(i=0; i<n; i+=r){
		r = lnx_syscall(LNX_SYS_READ,
			(unsigned long int[6]){
				fd,
				(unsigned long int)buf + i,
				n - i,
				0,
				0,
				0
			},
			1
		);

		if(r <= 0)
			LNX_SYSCALL_ERROR_EXIT("%d != %d\n", r, n);
	}
}

void lnx_write(int fd, void const *buf, size_t n){
	size_t r;


	r = lnx_syscall(LNX_SYS_WRITE,
		(unsigned long int[6]){
			fd,
			(unsigned long int)buf,
			n,
			0,
			0,
			0
		},
		1
	);

	if(r != n)
		LNX_SYSCALL_ERROR_EXIT("%d != %d\n", r, n);
}

void lnx_dprintf(int fd, char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	lnx_vdprintf(fd, fmt, lst);
	va_end(lst);
}

void lnx_vdprintf(int fd, char const *fmt, va_list lst){
	static FILE fp = FILE_INITIALISER(0x0, 0x0, 0, lnx_putc);


	fp.fileno = fd;

	(void)vfprintf(&fp, fmt, lst);
}


/* local functions */
static char lnx_putc(char c, struct FILE *stream){
	lnx_write(stream->fileno, &c, 1);

	return c;
}
