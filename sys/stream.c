/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/stdarg.h>
#include <sys/string.h>
#include <sys/stream.h>
#include <sys/limits.h>


/* macros */
#if defined(CONFIG_PRINTF_LONGLONG)
#define INTTYPE		long long int
#define UINTTYPE	unsigned long long int
#elif defined(CONFIG_PRINTF_LONG)
#define INTTYPE		long int
#define UINTTYPE	unsigned long int
#elif defined(CONFIG_PRINTF_INTMAX)
#define INTTYPE		intmax_t
#define UINTTYPE	uintmax_t
#elif defined(CONFIG_PRINTF_SIZET)
#define INTTYPE		ssize_t
#define UINTTYPE	size_t
#elif defined(CONFIG_PRINTF_PTRDIFF)
#define INTTYPE		long int
#define UINTTYPE	unsigned long int
#else
#define INTTYPE		int
#define UINTTYPE	unsigned int
#endif // CONFIG_PRINTF_* - INTTYPE

#if defined(CONFIG_PRINTF_LONGDOUBLE)
#define FLOATTYPE	long double
#elif !defined(CONFIG_NOFLOAT)
#define FLOATTYPE	double
#endif // CONFIG_PRINT_* - FLOATTYPE


/* types */
typedef enum{
	FFL_NONE = 0x0,
	FFL_UPPERCASE = 0x1,
	FFL_LEFTALIGN = 0x2,
	FFL_LEFTPADZERO = 0x4,
	FFL_FORCESIGN = 0x8,
	FFL_SIGNBLANK = 0x10,
	FFL_PREFIX = 0x20,
	FFL_SIGNED = 0x40,
	FFL_PRECISION = 0x80
} fflag_t;

typedef enum{
	LEN_CHAR = 1,
	LEN_SHORT,
	LEN_INT,
	LEN_LONG,
	LEN_LONGLONG,
	LEN_SIZET,
	LEN_PTRDIFF,
	LEN_INTMAX,
} len_t;

typedef union{
	UINTTYPE i;
	void *p;

#ifdef FLOATTYPE
	FLOATTYPE d;
#endif // FLOATTYPE
} value_t;


/* local/static prototypes */
static size_t parse_flags(char const *format, fflag_t *flags);
static size_t parse_num(char const *format, va_list lst, size_t *num);
static size_t parse_len(char const *format, len_t *len);

static int get_spec(char const *format, va_list lst, len_t len, fflag_t *flags, value_t *v);
static int get_arg(va_list lst, len_t len, bool check_sign, fflag_t *flags, value_t *v);

static size_t put_char(FILE *stream, char c);
static size_t put_spec(FILE *stream, char spec, char const *buf, size_t buf_len, bool buf_inv, fflag_t flags, size_t width, size_t prec);
static size_t put_padding(FILE *stream, char pad, size_t n);
static size_t put_buf(FILE *stream, char const *b, size_t n, bool inv);
static void put_len(len_t len, value_t *v, size_t n);

static size_t convert(char const *format, fflag_t flags, value_t *v, char *buf);
static size_t utoa_inv(UINTTYPE v, char *s, unsigned int base, fflag_t flags);


/* global functions */
int vfprintf(FILE *stream, char const *format, va_list lst){
	char const *fp;
	size_t width,
		   prec;
	size_t n,
		   blen;
	len_t len;
	fflag_t flags;
	value_t v;
#ifdef CONFIG_NOFLOAT
	char buf[23];
#else
	/* TODO */
	#warning "vfprintf floating point support not implemented"
	char buf[23];
#endif // CONFIG_NOFLOAT


	if(stream == 0x0 || (stream->putc == 0x0 && stream->wbuf == 0x0))
		return 0;

	n = 0;
	blen = 0;

	while(*format){
		if(*format != '%'){
			n += put_char(stream, *format);
			format++;

			continue;
		}
		else if(*format == '%' && *(format + 1) == '%'){
			n += put_char(stream, '%');
			format += 2;

			continue;
		}

		fp = ++format;

		memset(&v, 0x0, sizeof(v));

		/* parse flags */
		format += parse_flags(format, &flags);

		/* parse width */
		format += parse_num(format, lst, &width);

		/* parse precision */
		prec = 0;

		if(*format == '.'){
			format++;
			flags |= FFL_PRECISION;

			format += parse_num(format, lst, &prec);
		}

		/* parse length */
		format += parse_len(format, &len);

		/* parse specifier */
		if(get_spec(format, lst, len, &flags, &v) < 0)
			goto spec_err;

		blen = convert(format, flags, &v, buf);

		/* output specifier */
		if(*format == 'n'){
			put_len(len, &v, n);
		}
		else if(*format == 's'){
			blen = strlen(v.p);

			if((flags & FFL_PRECISION) && prec < blen)
				blen = prec;

			n += put_spec(stream, *format, v.p, blen, false, flags, width, 0);
		}
		else if(*format == 'c'){
			n += put_spec(stream, *format, buf, blen, false, flags, width, 0);
		}
		else{
			// ignore 0-flag if precision is specified
			if(flags & FFL_PRECISION)
				flags &= ~FFL_LEFTPADZERO;

			if(!((flags & FFL_PRECISION) && prec == 0 && *buf == '0'))
				n += put_spec(stream, *format, buf, blen, true, flags, width, prec);
		}

		format++;
		continue;


spec_err:
		n += put_char(stream, '%');
		format = fp;
	}

	// zero-terminate string streams
	if(stream->putc == 0x0)
		put_char(stream, 0);

	return n;
}


/* local functions */
static size_t parse_flags(char const *format, fflag_t *flags){
	size_t i;


	i = 0;
	*flags = FFL_NONE;

	while(1){
		switch(format[i]){
		case '-':
			*flags |= FFL_LEFTALIGN;
			break;

		case '+':
			*flags |= FFL_FORCESIGN;
			break;

		case ' ':
			*flags |= FFL_SIGNBLANK;
			break;

		case '0':
			*flags |= FFL_LEFTPADZERO;
			break;

		case '#':
			*flags |= FFL_PREFIX;
			break;

		default:
			return i;
		}

		i++;
	}
}

static size_t parse_num(char const *format, va_list lst, size_t *num){
	size_t i;


	i = 0;
	*num = 0;

	while(1){
		if(format[i] >= '0' && format[i] <= '9'){
			*num *= 10;
			*num += format[i] - '0';
		}
		else if(format[i] == '*'){
			*num = (unsigned int)(va_arg(lst, unsigned int));
			return i + 1;
		}
		else
			return i;

		i++;
	}
}

static size_t parse_len(char const *format, len_t *len){
	switch(*format++){
	case 'h':
		*len = LEN_SHORT;

		if(*format == 'h'){
			*len = LEN_CHAR;
			return 2;
		}

		break;

	case 'L':
		*len = LEN_LONGLONG;
		break;

	case 'l':
		*len = LEN_LONG;

		if(*format == 'l'){
			*len = LEN_LONGLONG;
			return 2;
		}

		break;

	case 'j':
		*len = LEN_INTMAX;
		break;

	case 'z':
		*len = LEN_SIZET;
		break;

	case 't':
		*len = LEN_PTRDIFF;
		break;

	default:
		*len = LEN_INT;
		return 0;
	}

	return 1;
}

static int get_spec(char const *format, va_list lst, len_t len, fflag_t *flags, value_t *v){
	switch(*format){
	case 'X':
		*flags |= FFL_UPPERCASE;
		// fall through

	case 'x': // fall through
	case 'u': // fall through
	case 'o':
		// ignore +-flag for unsigned specifier
		*flags &= ~FFL_FORCESIGN;

		return get_arg(lst, len, false, flags, v);

	case 'd': // fall through
	case 'i': // fall through
		return get_arg(lst, len, true, flags, v);

	case 'f': // fall through
	case 'F': // fall through
	case 'e': // fall through
	case 'E': // fall through
	case 'g': // fall through
	case 'G': // fall through
	case 'a': // fall through
	case 'A':
		switch(len){
		case LEN_LONGLONG:
			(void)va_arg(lst, long double);
			break;

		default:
			(void)va_arg(lst, double);
		}

		return -1;

	case 'c':
		v->i = va_arg(lst, int);
		break;

	case 'p':
		*flags |= FFL_PREFIX;
		// fall through

	case 's': // fall through
	case 'n':
		v->p = va_arg(lst, void*);
		break;
	}

	return 0;
}

static int get_arg(va_list lst, len_t len, bool check_sign, fflag_t *flags, value_t *v){
	UINTTYPE max;


	switch(len){
	case LEN_CHAR:
		v->i = va_arg(lst, int);
		max = UCHAR_MAX;
		break;

	case LEN_SHORT:
		v->i = va_arg(lst, int);
		max = USHRT_MAX;
		break;

	case LEN_INT:
		v->i = va_arg(lst, int);
		max = UINT_MAX;
		break;

	case LEN_LONG:
#ifdef CONFIG_PRINTF_LONG
		v->i = va_arg(lst, long int);
		max = ULONG_MAX;
		break;
#else
		(void)va_arg(lst, long int);
		return -1;
#endif // CONFIG_PRINTF_LONG

	case LEN_LONGLONG:
#ifdef CONFIG_PRINTF_LONGLONG
		v->i = va_arg(lst, long long int);
		max = ULLONG_MAX;
		break;
#else
		(void)va_arg(lst, long long int);
		return -1;
#endif // CONFIG_PRINTF_LONGLONG

	case LEN_SIZET:
#ifdef CONFIG_PRINTF_SIZET
		v->i = va_arg(lst, size_t);
		max = SIZE_MAX;
		break;
#else
		(void)va_arg(lst, size_t);
		return -1;
#endif // CONFIG_PRINTF_SIZET

	case LEN_PTRDIFF:
#ifdef CONFIG_PRINTF_PTRDIFF
		v->i = va_arg(lst, PTRDIFF_T);
		max = (UINTTYPE)PTRDIFF_MAX * 2 + 1;
		break;
#else
		(void)va_arg(lst, PTRDIFF_T);
		return -1;
#endif // CONFIG_PRINTF_PTRDIFF

	case LEN_INTMAX:
#ifdef CONFIG_PRINTF_INTMAX
		v->i = va_arg(lst, intmax_t);
		max = UINTMAX_MAX;
		break;
#else
		(void)va_arg(lst, intmax_t);
		return -1;
#endif // CONFIG_PRINTF_INTMAX

	default:
		return -1;
	}

	if(!check_sign){
		if((INTTYPE)v->i < 0)
			v->i = max + (INTTYPE)v->i + 1;
	}
	else if((INTTYPE)v->i < 0){
		*flags |= FFL_SIGNED;
		v->i *= -1;
	}

	return 0;
}

static size_t put_char(FILE *stream, char c){
	if(stream->putc)
		return (stream->putc(c, stream) == c) ? 1 : 0;

	if(stream->wbuf == 0x0 || stream->widx == stream->blen)
		return 0;

	((char*)stream->wbuf)[stream->widx++] = c;
	return 1;
}

static size_t put_spec(FILE *stream, char spec, char const *buf, size_t buf_len, bool buf_inv, fflag_t flags, size_t width, size_t prec){
	size_t i,
		   n,
		   n_sign,
		   n_prefix;
	char sign,
		 prefix[3];


	n = 0;

	/* handle sign */
	if(flags & FFL_SIGNED)			sign = '-';
	else if(flags & FFL_FORCESIGN)	sign = '+';
	else if(flags & FFL_SIGNBLANK)	sign = ' ';
	else							sign = 0;

	n_sign = sign ? 1 : 0;

	/* handle prefix */
	n_prefix = 0;
	prefix[2] = 0;

	if(flags & FFL_PREFIX){
		switch(spec){
		case 'x':
		case 'p':
			prefix[0] = '0';
			prefix[1] = 'x';
			n_prefix = 2;
			break;

		case 'X':
			prefix[0] = '0';
			prefix[1] = 'X';
			n_prefix = 2;
			break;

		case 'o':
			if(*buf != '0' && prec < buf_len){
				prefix[0] = '0';
				n_prefix = 1;
			}
			break;
		}
	}

	/* handle precision */
	if(prec < buf_len)
		prec = buf_len;

	/* left-pad spaces if not left-aligned */
	if(!(flags & FFL_LEFTALIGN) && width > prec + n_sign + n_prefix)
		n += put_padding(stream, (flags & FFL_LEFTPADZERO ? '0' : ' '), width - prec - n_sign - n_prefix);

	/* print sign */
	n += put_padding(stream, sign, n_sign);

	/* print prefix */
	for(i=0; i<n_prefix; i++)
		n += put_char(stream, prefix[i]);

	/* left-pad zeroes */
	n += put_padding(stream, '0', prec - buf_len);

	/* print number */
	n += put_buf(stream, buf, buf_len, buf_inv);

	/* right-pad spaces if left-aligned */
	if(flags & FFL_LEFTALIGN && width > prec + n_sign + n_prefix)
		n += put_padding(stream, ' ', width - prec - n_sign - n_prefix);

	return n;
}

static size_t put_padding(FILE *stream, char pad, size_t n){
	size_t i;


	for(i=0; i<n; i++){
		if(put_char(stream, pad) == 0)
			break;
	}

	return i;
}

static size_t put_buf(FILE *stream, char const *b, size_t n, bool inv){
	size_t i;
	int8_t dir;


	dir = 1;

	if(inv){
		b += n - 1;
		dir = -1;
	}

	for(i=0; i<n; i++){
		if(put_char(stream, *b) == 0)
			break;

		b += dir;
	}

	return i;
}

static void put_len(len_t len, value_t *v, size_t n){
	switch(len){
	case LEN_CHAR:
		*((char*)v->p) = n;
		break;

	case LEN_SHORT:
		*((short int*)v->p) = n;
		break;

	case LEN_INT:
		*((int*)v->p) = n;
		break;

#ifdef CONFIG_PRINTF_LONG
	case LEN_LONG:
		*((long int*)v->p) = n;
		break;
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	case LEN_LONGLONG:
		*((long long int*)v->p) = n;
		break;
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
	case LEN_SIZET:
		*((size_t*)v->p) = n;
		break;
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	case LEN_PTRDIFF:
		*((ptrdiff_t*)v->p) = n;
		break;
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
	case LEN_INTMAX:
		*((intmax_t*)v->p) = n;
		break;
#endif // CONFIG_PRINTF_INTMAX

	default:
		break;
	}
}

static size_t convert(char const *format, fflag_t flags, value_t *v, char *buf){
	switch(*format){
	case 'X': // fall through
	case 'x':
	case 'p':
		return utoa_inv(v->i, buf, 16, flags);

	case 'o':
		return utoa_inv(v->i, buf, 8, flags);

	case 'i': // fall through
	case 'd': // fall through
	case 'u':
		return utoa_inv(v->i, buf, 10, flags);

	case 'f': // fall through
	case 'F': // fall through
	case 'e': // fall through
	case 'E': // fall through
	case 'g': // fall through
	case 'G': // fall through
	case 'a': // fall through
	case 'A':
		/* TODO */
		return 0;

	case 'c':
		buf[0] = (char)v->i;
		return 1;
	}

	return 0;
}

static size_t utoa_inv(UINTTYPE v, char *s, unsigned int base, fflag_t flags){
	unsigned int i;
	char d;


	i = 0;

	do{
		d = (v % base) % 0xff;
		v /= base;

		if(d < 10)						s[i] = '0' + d;
		else if(flags & FFL_UPPERCASE)	s[i] = 'A' + d - 10;
		else							s[i] = 'a' + d - 10;

		i++;
	}while(v != 0);

	return i;
}
