#ifndef SYS_STDIO_H
#define SYS_STDIO_H


#include <sys/stdarg.h>


/* macros */
#define EOF		F_EOF


/* types */
typedef enum{
	F_EIO = -2,
	F_EOF = -1,
	F_OK = 0,
} fstate_t;

typedef struct{
	char *buf;
	unsigned int pos;

	fstate_t state;

	char (*putc)(char c);
	int (*puts)(char const *s);
} FILE;


/* prototypes */
char fputc(char c, FILE *stream);
int fputs(char const *s, FILE *stream);

int vfprintf(FILE *stream, char const *format, va_list lst);


#endif // SYS_STDIO_H
