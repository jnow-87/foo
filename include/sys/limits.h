/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_LIMITS_H
#define SYS_LIMITS_H


#include <config/config.h>
#include <sys/compiler.h>


/* macros */
#ifndef BUILD_HOST
#if CONFIG_REGISTER_WIDTH == 8

// signed types
#define CHAR_BIT		8
#define CHAR_MAX		SCHAR_MAX
#define CHAR_MIN		SCHAR_MIN
#define SCHAR_MAX		127
#define SCHAR_MIN		(-SCHAR_MAX - 1)
#define SHRT_MAX		32767
#define SHRT_MIN		(-SHRT_MAX - 1)
#define INT_MAX			32767
#define INT_MIN			(-INT_MAX - 1)
#define LONG_MAX		2147483647
#define LONG_MIN		(-LONG_MAX - 1)
#define LLONG_MAX		9223372036854775807
#define LLONG_MIN		(-LLONG_MAX - 1)
#define INTMAX_MAX		LLONG_MAX
#define INTMAX_MIN		LLONG_MIN
#define PTRDIFF_MAX		32767
#define PTRDIFF_MIN		(-PTRDIFF_MAX - 1)

// unsigned types
#define UCHAR_MAX		255U
#define USHRT_MAX		65535U
#define UINT_MAX		65535U
#define ULONG_MAX		4294967295U
#define ULLONG_MAX		18446744073709551615U
#define UINTMAX_MAX		ULLONG_MAX
#define SIZE_MAX		UINT_MAX

#elif CONFIG_REGISTER_WIDTH == 32 // CONFIG_REGISTER_WIDTH

// signed types
#define CHAR_BIT		8
#define CHAR_MAX		SCHAR_MAX
#define CHAR_MIN		SCHAR_MIN
#define SCHAR_MAX		127
#define SCHAR_MIN		(-SCHAR_MAX - 1)
#define SHRT_MAX		32767
#define SHRT_MIN		(-SHRT_MAX - 1)
#define INT_MAX			2147483647
#define INT_MIN			(-INT_MAX - 1)
#define LONG_MAX		9223372036854775807
#define LONG_MIN		(-LONG_MAX - 1)
#define LLONG_MAX		9223372036854775807
#define LLONG_MIN		(-LLONG_MAX - 1)
#define INTMAX_MAX		LLONG_MAX
#define INTMAX_MIN		LLONG_MIN
#define PTRDIFF_MAX		9223372036854775807
#define PTRDIFF_MIN		(-PTRDIFF_MAX - 1)

// unsigned types
#define UCHAR_MAX		255U
#define USHRT_MAX		65535U
#define UINT_MAX		4294967295U
#define ULONG_MAX		18446744073709551615U
#define ULLONG_MAX		18446744073709551615U
#define UINTMAX_MAX		ULLONG_MAX
#define SIZE_MAX		UINT_MAX

#else

CPP_ASSERT(invalid address width in configuration)

#endif // CONFIG_REGISTER_WIDTH

#else // BUILD_HOST

#include <limits.h>

#endif // BUILD_HOST


#endif // SYS_LIMITS_H
