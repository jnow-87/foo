#include <config/config.h>
#include <sys/stdarg.h>
#include <sys/string.h>
#include <sys/stream.h>


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
#define INTTYPE		sptrdiff_t
#define UINTTYPE	ptrdiff_t
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
#ifdef  CONFIG_PRINTF_LONG
	LEN_LONG,
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
	LEN_LONGLONG,
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
	LEN_SIZET,
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
	LEN_PTRDIFF,
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
	LEN_INTMAX,
#endif // CONFIG_PRINTF_INTMAX
} len_t;


/* local/static prototypes */
static size_t put_char(FILE *stream, char c);
static size_t put_spec(FILE *stream, char spec, char const *buf, size_t buf_len, bool buf_inv, fflag_t flags, size_t width, size_t prec);
static size_t put_padding(FILE *stream, char pad, size_t n);
static size_t put_buf(FILE *stream, char const *b, size_t n, bool inv);
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
	union{
		INTTYPE ref;
		char c;
		short int s;
		int i;
		void *p;

#ifdef CONFIG_PRINTF_LONG
		long int l;
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
		long long int ll;
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_INTMAX
		intmax_t im;
#endif // CONFIG_PRINTF_INTMAX

#ifdef CONFIG_PRINTF_SIZET
		ssize_t st;
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
		sptrdiff_t pd;
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef FLOATTYPE
		FLOATTYPE d;
#endif // FLOATTYPE
	} v;

#ifdef CONFIG_NOFLOAT
	char buf[23];
#else
	/* TODO */
	#warning "vfprintf floating point support not implemented"
	char buf[23];
#endif // CONFIG_NOFLOAT


	if(stream->putc == 0x0 && stream->wbuf == 0x0)
		return 0;

	n = 0;
	blen = 0;

	while(*format){
		if(*format != '%'){
			n += put_char(stream, *format++);
			continue;
		}

		fp = ++format;


		memset(&v, 0x0, sizeof(v));

		width = 0;
		prec = 0;
		flags = FFL_NONE;
		len = LEN_INT;

		/* parse flags */
		while(1){
			switch(*format++){
			case '%':
				n += put_char(stream, '%');
				goto lcontinue;

			case '-':
				flags |= FFL_LEFTALIGN;
				break;

			case '+':
				flags |= FFL_FORCESIGN;
				break;

			case ' ':
				flags |= FFL_SIGNBLANK;
				break;

			case '0':
				flags |= FFL_LEFTPADZERO;
				break;

			case '#':
				flags |= FFL_PREFIX;
				break;

			default:
				--format;
				goto parse_width;
			}
		}

		/* parse width */
parse_width:
		while(1){
			if(*format >= '0' && *format <= '9'){
				width *= 10;
				width += *format - '0';
				format++;
			}
			else if(*format == '*'){
				width = (unsigned int)(va_arg(lst, unsigned int));
				format++;
				break;
			}
			else
				break;
		}

		/* parse precision */
		if(*format == '.'){
			format++;
			flags |= FFL_PRECISION;

			while(1){
				if(*format >= '0' && *format <= '9'){
					prec *= 10;
					prec += *format - '0';
					format++;
				}
				else if(*format == '*'){
					prec = (unsigned int)(va_arg(lst, unsigned int));
					format++;
					break;
				}
				else
					break;
			}
		}

		/* parse length */
		switch(*format++){
		case 'h':
			len = LEN_SHORT;

			if(*format == 'h'){
				len = LEN_CHAR;
				format++;
			}

			break;

		case 'L':
#ifdef CONFIG_PRINTF_LONGLONG
			len = LEN_LONGLONG;
			break;
#else
			goto spec_err;
#endif // CONFIG_PRINTF_LONGLONG

		case 'l':
#ifdef CONFIG_PRINTF_LONG
			len = LEN_LONG;

			if(*format == 'l'){
#ifdef CONFIG_PRINTF_LONGLONG
				len = LEN_LONGLONG;
				format++;
#else
				goto spec_err;
#endif // CONFIG_PRINTF_LONGLONG
			}

			break;
#else
			goto spec_err;
#endif // CONFIG_PRINTF_LONG

		case 'j':
#ifdef CONFIG_PRINTF_INTMAX
			len = LEN_INTMAX;
			break;
#else
			goto spec_err;
#endif // CONFIG_PRINTF_INTMAX

		case 'z':
#ifdef CONFIG_PRINTF_SIZET
			len = LEN_SIZET;
			break;
#else
			goto spec_err;
#endif // CONFIG_PRINTF_SIZET

		case 't':
#ifdef CONFIG_PRINTF_PTRDIFF
			len = LEN_PTRDIFF;
			break;
#else
			goto spec_err;
#endif // CONFIG_PRINTF_PTRDIFF

		default:
			--format;
			break;
		}

		/* parse specifier */
		switch(*format){
		case 'X':
			flags |= FFL_UPPERCASE;
			// fall through

		case 'x': // fall through
		case 'u': // fall through
		case 'o':
			// ignore +-flag for unsigned specifier
			flags &= -1 ^ FFL_FORCESIGN;

			switch(len){
			case LEN_CHAR:
				v.c = va_arg(lst, int);
				break;

			case LEN_SHORT:
				v.s = va_arg(lst, int);
				break;

			case LEN_INT:
				v.i = va_arg(lst, int);
				break;

#ifdef CONFIG_PRINTF_LONG
			case LEN_LONG:
				v.l = va_arg(lst, long int);
				break;
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
			case LEN_LONGLONG:
				v.ll = va_arg(lst, long long int);
				break;
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
			case LEN_SIZET:
				v.st = va_arg(lst, size_t);
				break;
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
			case LEN_PTRDIFF:
				v.pd = va_arg(lst, PTRDIFF_T);
				break;
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
			case LEN_INTMAX:
				v.im = va_arg(lst, intmax_t);
				break;
#endif // CONFIG_PRINTF_INTMAX
			}

			break;

		case 'd': // fall through
		case 'i': // fall through
			switch(len){
			case LEN_CHAR:	// fall through
			case LEN_SHORT:	// fall through
			case LEN_INT:
				v.i = va_arg(lst, int);

				if(v.i < 0){
					flags |= FFL_SIGNED;
					v.i *= -1;
				}

				break;

#ifdef CONFIG_PRINTF_LONG
			case LEN_LONG:
				v.l = va_arg(lst, long int);

				if(v.l < 0){
					flags |= FFL_SIGNED;
					v.l *= -1;
				}

				break;
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
			case LEN_LONGLONG:
				v.ll = va_arg(lst, long long int);

				if(v.ll < 0){
					flags |= FFL_SIGNED;
					v.ll *= -1;
				}

				break;
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
			case LEN_SIZET:
				v.st = va_arg(lst, size_t);

				if(v.st < 0){
					flags |= FFL_SIGNED;
					v.st *= -1;
				}

				break;
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
			case LEN_PTRDIFF:
				v.pd = va_arg(lst, PTRDIFF_T);

				if(v.pd < 0){
					flags |= FFL_SIGNED;
					v.pd *= -1;
				}

				break;
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
			case LEN_INTMAX:
				v.im = va_arg(lst, intmax_t);

				if(v.im < 0){
					flags |= FFL_SIGNED;
					v.im *= -1;
				}

				break;
#endif // CONFIG_PRINTF_INTMAX
			}

			break;

		case 'f': // fall through
		case 'F': // fall through
		case 'e': // fall through
		case 'E': // fall through
		case 'g': // fall through
		case 'G': // fall through
		case 'a': // fall through
		case 'A':
			goto spec_err;

		case 'c':
			v.i = va_arg(lst, int);
			break;

		case 'p':
			flags |= FFL_PREFIX;
			// fall through

		case 's': // fall through
		case 'n':
			v.p = va_arg(lst, void*);
			break;
		}

		switch(*format){
		case 'X': // fall through
		case 'x':
		case 'p':
			blen = utoa_inv(v.ref, buf, 16, flags);
			break;

		case 'o':
			blen = utoa_inv(v.ref, buf, 8, flags);
			break;

		case 'i': // fall through
		case 'd': // fall through
		case 'u':
			blen = utoa_inv(v.ref, buf, 10, flags);
			break;

		case 'f': // fall through
		case 'F': // fall through
		case 'e': // fall through
		case 'E': // fall through
		case 'g': // fall through
		case 'G': // fall through
		case 'a': // fall through
		case 'A':
			/* TODO */
			break;

		case 'c':
			blen = 1;
			buf[0] = v.c;
			break;
		}

		/* output specifier */
		if(*format == 'n'){
			switch(len){
			case LEN_CHAR:
				*((char*)v.p) = n;
				break;

			case LEN_SHORT:
				*((short int*)v.p) = n;
				break;

			case LEN_INT:
				*((int*)v.p) = n;
				break;

#ifdef CONFIG_PRINTF_LONG
			case LEN_LONG:
				*((long int*)v.p) = n;
				break;
#endif // CONFIG_PRINTF_LONG

#ifdef CONFIG_PRINTF_LONGLONG
			case LEN_LONGLONG:
				*((long long int*)v.p) = n;
				break;
#endif // CONFIG_PRINTF_LONGLONG

#ifdef CONFIG_PRINTF_SIZET
			case LEN_SIZET:
				*((size_t*)v.p) = n;
				break;
#endif // CONFIG_PRINTF_SIZET

#ifdef CONFIG_PRINTF_PTRDIFF
			case LEN_PTRDIFF:
				*((ptrdiff_t*)v.p) = n;
				break;
#endif // CONFIG_PRINTF_PTRDIFF

#ifdef CONFIG_PRINTF_INTMAX
			case LEN_INTMAX:
				*((intmax_t*)v.p) = n;
				break;
#endif // CONFIG_PRINTF_INTMAX
			}
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
				flags &= -1 ^ FFL_LEFTPADZERO;

			if(!((flags & FFL_PRECISION) && prec == 0 && *buf == '0'))
				n += put_spec(stream, *format, buf, blen, true, flags, width, prec);
		}

		format++;

lcontinue:
		continue;

spec_err:
		n += put_char(stream, '%');
		format = fp;
	}

	return n;
}

/* local functions */
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
