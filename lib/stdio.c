#include <config/config.h>
#include <lib/init.h>
#include <lib/stdlib.h>
#include <lib/unistd.h>
#include <lib/stdio.h>
#include <sys/stream.h>
#include <sys/string.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/math.h>
#include <sys/limits.h>


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
	file = malloc(sizeof(FILE));

	if(file == 0x0)
		goto_errno(err_0, E_NOMEM);

	memset(file, 0x0, sizeof(FILE));

	file->fd = fd;
	file->blen = CONFIG_FILE_BUF_SIZE;

	if(fmode & O_READ){
		file->rbuf = malloc(CONFIG_FILE_BUF_SIZE);
		file->ridx = 0;
		file->dataidx = 0;

		if(file->rbuf == 0x0)
			goto_errno(err_1, E_NOMEM);
	}

	if(fmode & O_WRITE){
		file->wbuf = malloc(CONFIG_FILE_BUF_SIZE);
		file->putc = fputc;

		if(file->wbuf == 0x0)
			goto_errno(err_1, E_NOMEM);
	}

	return file;


err_1:
	free(file->rbuf);
	free(file->wbuf);
	free(file);

err_0:
	return 0x0;
}

int fclose(FILE *stream){
	if(fflush(stream) != 0)
		goto err;

	if(close(stream->fd) != E_OK)
		goto err;

	free(stream->rbuf);
	free(stream->wbuf);
	free(stream);

	return 0;


err:
	return EOF;
}

size_t fread(void *p, size_t size, FILE *stream){
	size_t n,
		   rd;
	ssize_t r;


	if(stream->rbuf == 0x0)
		goto_errno(err_0, E_INVAL);

	rd = 0;

	while(1){
		/* copy data from stream read buffer */
		n = MIN(stream->dataidx - stream->ridx, size - rd);

		memcpy(p + rd, stream->rbuf + stream->ridx, n);
		stream->ridx += n;
		rd += n;

		if(size - rd == 0)
			return rd;

		/* read data from file system */
		if(size - rd < stream->blen){
			r = read(stream->fd, stream->rbuf, stream->blen);

			if(r <= 0)
				goto err_1;

			stream->dataidx = r;
			stream->ridx = 0;
		}
		else{
			r = read(stream->fd, p + rd, size - rd);

			if(r < 0)
				goto err_0;

			return rd + r;
		}
	}


err_1:
	stream->dataidx = 0;
	stream->ridx = 0;

err_0:

	return 0;
}

size_t fwrite(void const *p, size_t size, FILE *stream){
	size_t n;
	ssize_t r;


	if(stream->wbuf == 0x0)
		goto_errno(err, E_INVAL);

	/* copy data to stream write buffer */
	if(size < stream->blen - stream->widx){
		memcpy(stream->wbuf + stream->widx, p, size);
		stream->widx += size;

		for(n=0; n<size; n++){
			if(((char*)p)[n] == '\n'){
				if(fflush(stream) != 0)
					goto err;
			}
		}

		return size;
	}

	/* write data to file system */
	if(fflush(stream) != 0)
		goto err;

	r = write(stream->fd, (void*)p, size);

	if(r < 0)
		goto err;

	return r;


err:
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
	if(stream->widx > 0){
		if(write(stream->fd, stream->wbuf, stream->widx) < 0)
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

	if(fcntl(stream->fd, F_SEEK, &p, sizeof(seek_t)) != E_OK)
		return -1;

	return 0;
}

int ftell(FILE *stream){
	seek_t p;


	if(fflush(stream) != 0)
		return -1;

	fcntl(stream->fd, F_TELL, &p, sizeof(seek_t));
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

int sprintf(char *s, char const *format, ...){
	int i;
	va_list lst;


	va_start(lst, format);
	i = vsprintf(s, format, lst);
	va_end(lst);

	return i;
}

int snprintf(char *s, size_t n, char const *format, ...){
	int i;
	va_list lst;


	va_start(lst, format);
	i = vsnprintf(s, n, format, lst);
	va_end(lst);

	return i;
}

int vprintf(char const *format, va_list lst){
	return vfprintf(stdout, format, lst);
}

int vsprintf(char *s, char const *format, va_list lst){
	return vsnprintf(s, SIZE_MAX, format, lst);
}

int vsnprintf(char *s, size_t n, char const *format, va_list lst){
	FILE fp = FILE_INITIALISER(0x0, s, n, 0x0);


	return vfprintf(&fp, format, lst);
}


/* local functions */
static int init(void){
	stdin = fopen("/dev/tty0", "r");
	stdout = fopen("/dev/tty0", "w");
	stderr = fopen("/dev/tty0", "w");

	return errno;
}

lib_init(init);

static f_mode_t mode_parse(char const *mode){
	f_mode_t fmode;


	fmode = 0;

	while(*mode){
		switch(*mode){
		case 'r':
			fmode |= O_READ;
			break;

		case 'w':
			fmode |= O_WRITE | O_CREATE;
			break;

		case 'a':
			fmode |= O_APPEND | O_CREATE;
			break;

		case '+':
			fmode |= O_READ | O_WRITE;
			break;
		}

		mode++;
	}

	return fmode;
}
