#ifndef SYS_STDIO_H
#define SYS_STDIO_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/stdarg.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/mutex.h>


/* macros */
#define EOF		E_END

#define FILE_INITIALISER(read_buf, write_buf, buf_len, putchar){ \
	.fileno = 0, \
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

#ifndef _x86_
#ifndef __x86_64__

#if defined(BUILD_KERNEL) && !defined(CONFIG_KERNEL_PRINTF)
#define vfprintf(stream, format, lst)	CALL_DISABLED(vfprintf, CONFIG_KERNEL_PRINTF)
#elif !defined(BUILD_KERNEL) && !defined(CONFIG_LIB_STREAM)
#define vfprintf(stream, format, lst)	CALL_DISABLED(vfprintf, CONFIG_LIB_STREAM)
#endif

#endif // __x86_64__
#endif // _x86_


/* types */
typedef struct FILE{
	int fileno;

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
#if (defined(BUILD_KERNEL) && defined(CONFIG_KERNEL_PRINTF)) \
 || (!defined(BUILD_KERNEL) && defined(CONFIG_LIB_STREAM)) \
 || defined(_x86_) \
 || defined(__x86_64)

int vfprintf(FILE *stream, char const *format, va_list lst);

#endif


#endif // SYS_STDIO_H
