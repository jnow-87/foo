/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <lib/init.h>
#include <lib/stdlib.h>
#include <lib/unistd.h>
#include <lib/stdio.h>
#include <sys/stream.h>
#include <sys/string.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/math.h>
#include <sys/mutex.h>


/* local/static prototypes */
static f_mode_t mode_parse(char const *mode);


/* global variables */
FILE *stdin = 0x0,
	 *stdout = 0x0,
	 *stderr = 0x0;


/* global functions */
FILE *fopen(char const *path, char const *mode){
	int fd;
	FILE *file;
	f_mode_t fmode;


	if(path == 0x0 || mode == 0x0)
		goto_errno(err_0, E_INVAL);

	fmode = mode_parse(mode);

	fd = open(path, fmode);

	if(fd < 0)
		goto err_0;

	file = fdopen(fd, mode);

	if(file == 0x0)
		goto err_1;

	return file;


err_1:
	close(fd);

err_0:
	return 0x0;
}

FILE *fdopen(int fd, char const *mode){
	f_mode_t fmode;
	FILE *file;


	if(mode == 0x0)
		goto_errno(err_0, E_INVAL);

	fmode = mode_parse(mode);

	/* allocate base struct and rw buffers */
	file = calloc(1, sizeof(FILE));

	if(file == 0x0)
		goto_errno(err_0, E_NOMEM);

	file->fileno = fd;
	file->blen = CONFIG_FILE_BUF_SIZE;

	if(fmode & O_RDONLY){
		file->rbuf = malloc(CONFIG_FILE_BUF_SIZE);
		file->ridx = 0;
		file->dataidx = 0;

		if(file->rbuf == 0x0)
			goto_errno(err_1, E_NOMEM);
	}

	if(fmode & O_WRONLY){
		file->wbuf = malloc(CONFIG_FILE_BUF_SIZE);
		file->putc = fputc;

		if(file->wbuf == 0x0)
			goto_errno(err_1, E_NOMEM);
	}

	mutex_init(&file->rd_mtx, MTX_NONE);
	mutex_init(&file->wr_mtx, MTX_NONE);

	return file;


err_1:
	free(file->rbuf);
	free(file->wbuf);
	free(file);

err_0:
	return 0x0;
}

int fileno(FILE *stream){
	return (stream == 0x0 ? -E_INVAL : stream->fileno);
}

int fclose(FILE *stream){
	if(stream == 0x0)
		goto err;

	if(fflush(stream) != 0)
		goto err;

	if(close(stream->fileno) != 0)
		goto err;

	free(stream->rbuf);
	free(stream->wbuf);
	free(stream);

	return 0;


err:
	return EOF;
}

size_t fread(void *p, size_t size, FILE *stream){
	size_t rd = 0;
	size_t n;
	ssize_t r;


	if(stream == 0x0 || stream->rbuf == 0x0)
		goto_errno(err_0, E_INVAL);

	mutex_lock(&stream->rd_mtx);

	while(1){
		/* copy data from stream read buffer */
		n = MIN(stream->dataidx - stream->ridx, size - rd);

		memcpy(p + rd, stream->rbuf + stream->ridx, n);
		stream->ridx += n;
		rd += n;

		if(size - rd == 0)
			break;

		/* read data from file system */
		if(size - rd < stream->blen){
			r = read(stream->fileno, stream->rbuf, stream->blen);

			stream->dataidx = r;
			stream->ridx = 0;

			if(r < 0){
				stream->dataidx = 0;
				goto err_1;
			}

			if(r == 0)
				break;
		}
		else{
			r = read(stream->fileno, p + rd, size - rd);

			if(r < 0)
				goto err_1;

			if(r == 0)
				break;

			rd += r;
			break;
		}
	}

	mutex_unlock(&stream->rd_mtx);

	return rd;


err_1:
	mutex_unlock(&stream->rd_mtx);

err_0:
	return 0;
}

size_t fwrite(void const *p, size_t size, FILE *stream){
	ssize_t r;


	if(stream == 0x0 || stream->wbuf == 0x0)
		goto_errno(err, E_INVAL);

	mutex_lock(&stream->wr_mtx);

	if(size < stream->blen - stream->widx){
		/* copy data to stream write buffer */
		memcpy(stream->wbuf + stream->widx, p, size);
		stream->widx += size;

		for(size_t n=0; n<size; n++){
			if(((char*)p)[n] == '\n'){
				if(fflush(stream) != 0)
					goto err;
			}
		}
	}
	else{
		/* write data to file system */
		if(fflush(stream) != 0)
			goto err;

		r = write(stream->fileno, (void*)p, size);
		size = r;

		if(r < 0)
			goto err;
	}

	mutex_unlock(&stream->wr_mtx);

	return size;


err:
	mutex_unlock(&stream->wr_mtx);

	return 0;
}

int fgetc(FILE *stream){
	char c;


	if(fread(&c, 1, stream) == 0)
		return EOF;

	return c;
}

char *fgets(char *s, int size, FILE *stream){
	if(fread(s, size, stream) == 0)
		return 0x0;

	return s;
}

char fputc(char c, FILE *stream){
	if(fwrite(&c, 1, stream) != 1)
		return EOF;

	return c;
}

int fputs(char const *s, FILE *stream){
	return fwrite(s, strlen(s), stream);
}

int fflush(FILE *stream){
	if(stream == 0x0)
		return EOF;

	if(stream->widx > 0){
		if(write(stream->fileno, stream->wbuf, stream->widx) < 0)
			return EOF;

		stream->widx = 0;
	}

	return 0;
}

int fseek(FILE *stream, int offset, whence_t whence){
	seek_t p;


	if(fflush(stream) != 0)
		return -1;

	p.whence = whence;
	p.offset = offset;

	if(fcntl(stream->fileno, F_SEEK, &p, sizeof(seek_t)) != 0)
		return -1;

	return 0;
}

int ftell(FILE *stream){
	seek_t p;


	if(fflush(stream) != 0)
		return -1;

	fcntl(stream->fileno, F_TELL, &p, sizeof(seek_t));
	return p.pos;
}

int printf(char const *format, ...){
	int i;
	va_list lst;


	va_start(lst, format);
	i = vprintf(format, lst);
	va_end(lst);

	return i;
}

int fprintf(FILE *stream, char const *format, ...){
	int i;
	va_list lst;


	va_start(lst, format);
	i = vfprintf(stream, format, lst);
	va_end(lst);

	return i;

}

int vprintf(char const *format, va_list lst){
	return vfprintf(stdout, format, lst);
}


/* local functions */
static int init(void){
	stdin = fopen("/dev/tty0", "r");
	stdout = fopen("/dev/tty0", "w");
	stderr = fopen("/dev/tty0", "w");

	return -errno;
}

lib_init(1, init);

static f_mode_t mode_parse(char const *mode){
	f_mode_t fmode = 0;


	while(*mode){
		switch(*mode){
		case 'r':
			fmode |= O_RDONLY;
			break;

		case 'w':
			fmode |= O_WRONLY | O_CREAT;
			break;

		case 'a':
			fmode |= O_APPEND | O_CREAT;
			break;

		case '+':
			fmode |= O_RDWR;
			break;
		}

		mode++;
	}

	return fmode;
}
