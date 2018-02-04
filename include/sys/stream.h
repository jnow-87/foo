#ifndef SYS_STDIO_H
#define SYS_STDIO_H


#include <sys/types.h>
#include <sys/stdarg.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/mutex.h>


/* macros */
#define EOF		E_END

#define FILE_INITIALISER(read_buf, write_buf, buf_len, putchar){ \
	.fd = 0, \
	.rbuf = read_buf, \
	.wbuf = write_buf, \
	.ridx = 0, \
	.dataidx = 0, \
	.widx = 0, \
	.blen = buf_len, \
	.rd_mtx = MUTEX_INITIALISER(), \
	.wr_mtx = MUTEX_INITIALISER(), \
	.putc = putchar, \
}


/* types */
typedef struct FILE{
	int fd;

	void *rbuf,
		 *wbuf;

	size_t ridx,
		   dataidx,
		   widx,
		   blen;

	mutex_t rd_mtx,
			wr_mtx;

	char (*putc)(char c, struct FILE *stream);
} FILE;


/* prototypes */
int vfprintf(FILE *stream, char const *format, va_list lst);


#endif // SYS_STDIO_H
