/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/compiler.h>
#include <sys/stdarg.h>
#include <sys/types.h>
#include <sys/stream.h>


/* macros */
#define CHECK_FP(stream){ \
	if(stream != &gcov_fp) \
		LNX_EEXIT("expecting gcov_fp\n"); \
}


/* prototypes */
int gcov_open(char const *path, int flags, int mode);
int gc_o(char const *path, int flags, int mode) __alias(gcov_open);
int gcov_close(int fd);
int gc_cl(int fd) __alias(gcov_close);

FILE *gcov_fdopen(int fd, char const *mode);
FILE *gc_fdo(int fd, char const *mode) __alias(gcov_fdopen);
FILE *gcov_fopen(char const *path, char const *mode);
FILE *gc_fo(char const *path, char const *mode) __alias(gcov_fopen);
int gcov_fclose(FILE *stream);
int gc_fcl(FILE *stream) __alias(gcov_fclose);

size_t gcov_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t gc_fr(void *ptr, size_t size, size_t nmemb, FILE *stream) __alias(gcov_fread);
size_t gcov_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t gc_fwr(const void *ptr, size_t size, size_t nmemb, FILE *stream) __alias(gcov_fwrite);

int gcov_fseek(FILE *stream, long offset, int whence);
int gc_fs(FILE *stream, long offset, int whence) __alias(gcov_fseek);
long gcov_ftell(FILE *stream);
long gc_ft(FILE *stream) __alias(gcov_ftell);
int gcov_fcntl(int fd, int cmd, ...);
int gc_fc(int fd, int cmd, ...) __alias(gcov_fcntl);
int gcov_access(char const *path, int mode);
int gc_acc(char const *path, int mode) __alias(gcov_access);

void gcov_setbuf(FILE *stream, char *buf);
void gc_sbf(FILE *stream, char *buf) __alias(gcov_setbuf);

int gcov_fprintf(FILE *stream, char const *format, ...);
int gc_fptf(FILE *stream, char const *format, ...) __alias(gcov_fprintf);
int gcov_vfprintf(FILE *stream, char const *format, va_list ap);
int vgc_fptf(FILE *stream, char const *format, va_list ap) __alias(gcov_vfprintf);


/* global variables */
// attribute comment required for renaming symbols
// in the original libgcov
//
// FILE gc_err __alias(gcov_stderr);
FILE gc_err = { 0 };


/* static variables */
static FILE gcov_fp = { 0 };
static bool gcov_fp_inuse = false;


/* global functions */
int gcov_open(char const *path, int flags, int mode){
	return lnx_open(path, flags, mode);
}

int gcov_close(int fd){
	lnx_close(fd);

	return 0;
}

FILE *gcov_fdopen(int fd, char const *mode){
	if(gcov_fp_inuse)
		LNX_EEXIT("expecting no nested gcov_fdopen calls\n");

	gcov_fp.fileno = fd;
	gcov_fp_inuse = true;

	return &gcov_fp;
}

FILE *gcov_fopen(char const *path, char const *mode){
	LNX_EEXIT("not expected to be called\n");

	return 0x0;
}

int gcov_fclose(FILE *stream){
	CHECK_FP(stream);

	gcov_fp_inuse = false;

	return gcov_close(stream->fileno);
}

size_t gcov_fread(void *ptr, size_t size, size_t nmemb, FILE *stream){
	ssize_t r;


	CHECK_FP(stream);

	r = lnx_read(stream->fileno, ptr, size * nmemb);

	return r;
}

size_t gcov_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream){
	CHECK_FP(stream);

	lnx_write(stream->fileno, ptr, size * nmemb);

	return nmemb;
}

int gcov_fseek(FILE *stream, long offset, int whence){
	long int r;


	CHECK_FP(stream);

	r = lnx_lseek(stream->fileno, offset, whence);

	return r;
}

long gcov_ftell(FILE *stream){
	long int r;


	CHECK_FP(stream);

	r = lnx_lseek(stream->fileno, 0, 1);

	return r;
}

int gcov_fcntl(int fd, int cmd, ...){
	if(cmd != 7)
		LNX_EEXIT("expecting F_SETLKW\n");

	return 0;
}

int gcov_access(char const *path, int mode){
	LNX_EEXIT("not expected to be called\n");

	return -1;
}

void gcov_setbuf(FILE *stream, char *buf){
	CHECK_FP(stream);

	if(buf != 0)
		LNX_EEXIT("expecting unbuffered mode\n");
}

int gcov_fprintf(FILE *stream, char const *format, ...){
	int r;
	va_list lst;


	va_start(lst, format);
	r = lnx_vdprintf(2, format, lst);
	va_end(lst);

	return r;
}

int gcov_vfprintf(FILE *stream, char const *format, va_list ap){
	return lnx_vdprintf(2, format, ap);
}
