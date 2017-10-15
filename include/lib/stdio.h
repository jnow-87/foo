#ifndef LIB_STDIO_H
#define LIB_STDIO_H


#include <sys/stream.h>
#include <sys/stdarg.h>
#include <sys/fcntl.h>


/* global variables */
extern FILE *stdin,
			*stdout,
			*stderr;


/* prototypes */
// FILE operations
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

// printf
int printf(char const *format, ...);
int fprintf(FILE *stream, char const *format, ...);
int sprintf(char *s, char const *format, ...);
int snprintf(char *s, size_t n, char const *format, ...);

int vprintf(char const *format, va_list lst);
int vsprintf(char *s, char const *format, va_list lst);
int vsnprintf(char *s, size_t n, char const *format, va_list lst);


#endif // LIB_STDIO_H
