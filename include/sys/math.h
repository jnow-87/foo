/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_MATH_H
#define SYS_MATH_H


#ifndef BUILD_HOST
# include <config/config.h>
#else
# include <math.h>
#endif // BUILD_HOST


/* macros */
#define ABS(v)({ \
	typeof(v) _v = v; \
	\
	\
	(_v > 0) ? _v : -_v; \
})

#define CMP(x, y, op)({ \
	typeof(x) _x = x; \
	typeof(y) _y = y; \
	\
	\
	(_x op _y) ? _x : _y; \
})

#define MIN(x, y)	CMP(x, y, <)
#define MAX(x, y)	CMP(x, y, >)

#define SQUARE(x)({ \
	typeof(x) _x = x; \
	\
	\
	_x * _x; \
})

// requires base to be power of 2
#define ALIGNP2(x, base)({ \
	typeof(x) _x = x; \
	typeof(base) _base = base; \
	\
	\
	(_x + _base - 1) & (~(_base - 1)); \
})


/* prototypes */
#ifndef BUILD_HOST
unsigned int log(unsigned int x, unsigned long base);
unsigned long long pow(unsigned long long x, unsigned long long y);
unsigned int powi(unsigned int x, unsigned int y);

# if !defined(CONFIG_NOFLOAT) || CONFIG_NOFLOAT == 0
double powd(double x, double y);
float powf(float x, float y);
long double powl(long double x, long double y);
# endif // CONFIG_NOFLOAT
#endif // BUILD_HOST

#endif // SYS_MATH_H
