#ifndef LIB_STDIO_H
#define LIB_STDIO_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/stream.h>
#include <sys/stdarg.h>
#include <sys/fcntl.h>


/* macros */
#ifndef CONFIG_SC_FILESYSTEM

#define fopen(path, mode)				(FILE*)CALL_DISABLED(fopen, CONFIG_SC_FILESYSTEM)
#define fdopen(fd, mode)				(FILE*)CALL_DISABLED(fdopen, CONFIG_SC_FILESYSTEM)
#define fclose(stream)					CALL_DISABLED(fclose, CONFIG_SC_FILESYSTEM)

#define fread(p, size, stream)			CALL_DISABLED(fread, CONFIG_SC_FILESYSTEM)
#define fwrite(p, size, stream)			CALL_DISABLED(fwrite, CONFIG_SC_FILESYSTEM)

#define fgetc(stream)					CALL_DISABLED(fgetc, CONFIG_SC_FILESYSTEM)
#define fgets(s, size, stream)			CALL_DISABLED(fgets, CONFIG_SC_FILESYSTEM)

#define fputc(c, stream)				CALL_DISABLED(fputc, CONFIG_SC_FILESYSTEM)
#define fputs(s, stream)				(char*)CALL_DISABLED(fputs, CONFIG_SC_FILESYSTEM)

#define fflush(stream)					CALL_DISABLED(fflush, CONFIG_SC_FILESYSTEM)
#define fseek(stream, offset, whence)	CALL_DISABLED(fseek, CONFIG_SC_FILESYSTEM)
#define ftell(stream)					CALL_DISABLED(ftell, CONFIG_SC_FILESYSTEM)

#endif // CONFIG_SC_FILESYSTEM

#ifndef CONFIG_LIB_PRINTF

#define printf(format, ...)				CALL_DISABLED(printf, CONFIG_LIB_PRINTF)
#define fprintf(stream, format, ...)	CALL_DISABLED(fprintf, CONFIG_LIB_PRINTF)
#define vprintf(format, lst)			CALL_DISABLED(vprintf, CONFIG_LIB_PRINTF)

#endif // CONFIG_LIB_PRINTF


/* global variables */
#ifdef CONFIG_SC_FILESYSTEM

extern FILE *stdin,
			*stdout,
			*stderr;

#endif // CONFIG_SC_FILESYSTEM


/* prototypes */
#ifdef CONFIG_SC_FILESYSTEM

FILE *fopen(char const *path, char const *mode);
FILE *fdopen(int fd, char const *mode);
int fclose(FILE *stream);

size_t fread(void *p, size_t size, FILE *stream);
size_t fwrite(void const *p, size_t size, FILE *stream);

int fgetc(FILE *stream);
char *fgets(char *s, int size, FILE *stream);

char fputc(char c, FILE *stream);
int fputs(char const *s, FILE *stream);

int fflush(FILE *stream);
int fseek(FILE *stream, int offset, whence_t whence);
int ftell(FILE *stream);

#endif // CONFIG_SC_FILESYSTEM

#ifdef CONFIG_LIB_PRINTF

int printf(char const *format, ...);
int fprintf(FILE *stream, char const *format, ...);
int vprintf(char const *format, va_list lst);

#endif // CONFIG_LIB_PRINTF


int sprintf(char *s, char const *format, ...);
int snprintf(char *s, size_t n, char const *format, ...);
int vsprintf(char *s, char const *format, va_list lst);
int vsnprintf(char *s, size_t n, char const *format, va_list lst);


#endif // LIB_STDIO_H
