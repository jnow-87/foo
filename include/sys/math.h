#ifndef SYS_MATH_H
#define SYS_MATH_H


#include <config/config.h>


/* macros */
#define abs(x) (((x) > 0) ? (x) : (-(x)))

// requires base to be power of 2
#define ALIGN(tgt, base) (((tgt) + (base) - 1) & ~((base) - 1))


/* prototypes */
unsigned int log(unsigned int x, unsigned long base);
unsigned long long pow(unsigned long long x, unsigned long long y);
unsigned int powi(unsigned int x, unsigned int y);

#if !defined(CONFIG_NOFLOAT) || CONFIG_NOFLOAT == 0

double powd(double x, double y);
float powf(float x, float y);
long double powl(long double x, long double y);

#endif // CONFIG_NOFLOAT


#endif // SYS_MATH_H
