/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_TYPES_H
#define SYS_TYPES_H


#include <sys/compiler.h>

#ifndef BUILD_HOST
# include <sys/devicetree.h>
#endif // BUILD_HOST


/* macros */
// boolean values
#define false	0
#define true	(!false)

// type name generation
#define TYPENAME(sign, type, width, ext) sign ##type ##width ##ext

#define INT(width)		TYPENAME(, int, width, _t)
#define UINT(width)		TYPENAME(u, int, width, _t)


/* types */
typedef _Bool							bool;
typedef void							*addr_t;

#ifndef BUILD_HOST
# if DEVTREE_ARCH_REG_WIDTH == 8
typedef signed int						int16_t;
typedef unsigned int					uint16_t;
typedef signed long int					int32_t;
typedef unsigned long int				uint32_t;
typedef unsigned int					size_t;
typedef int								ssize_t;
# elif DEVTREE_ARCH_REG_WIDTH == 32 || DEVTREE_ARCH_REG_WIDTH == 64
typedef signed short int				int16_t;
typedef unsigned short int				uint16_t;
typedef signed int						int32_t;
typedef unsigned int					uint32_t;
typedef unsigned long int				size_t;
typedef long int						ssize_t;
# else // DEVTREE_ARCH_REG_WIDTH
STATIC_ASSERT(!"invalid address width in configuration");
# endif // DEVTREE_ARCH_REG_WIDTH

typedef signed char						int8_t;
typedef unsigned char					uint8_t;
typedef signed long long int			int64_t;
typedef unsigned long long int			uint64_t;
typedef long long int					intmax_t;
typedef unsigned long long int			uintmax_t;

typedef INT(DEVTREE_ARCH_ADDR_WIDTH)	ptrdiff_t;
typedef UINT(DEVTREE_ARCH_REG_WIDTH)	register_t;
#else // BUILD_HOST
# include <inttypes.h>
# include <stddef.h>

typedef long int						ssize_t;
#endif // BUILD_HOST


#endif // SYS_TYPES_H
