/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_TYPES_H
#define SYS_TYPES_H


#include <config/config.h>
#include <sys/compiler.h>


/* macros */
// boolean values
#define false	0
#define true	(!false)

// type name generation
#define TYPENAME(sign, type, width, ext) \
	sign ##type ##width ##ext

#define INT(width)		TYPENAME(, int, width, _t)
#define UINT(width)		TYPENAME(u, int, width, _t)


/* architecture dependent types */
typedef char					bool;
typedef void					*addr_t;


/* fixed-width types */
#ifndef BUILD_HOST
# if CONFIG_REGISTER_WIDTH == 8
typedef signed char				int8_t;
typedef unsigned char			uint8_t;

typedef signed short int		int16_t;
typedef unsigned short int		uint16_t;

typedef signed long int			int32_t;
typedef unsigned long int		uint32_t;

typedef uint8_t					register_t;
typedef unsigned int			size_t;
typedef int						ssize_t;

typedef long long int			intmax_t;
typedef unsigned long long int	uintmax_t;

typedef int16_t					ptrdiff_t;
#  define PTRDIFF_T				unsigned int
# elif CONFIG_REGISTER_WIDTH == 32
typedef signed char				int8_t;
typedef unsigned char			uint8_t;

typedef signed short int		int16_t;
typedef unsigned short int		uint16_t;

typedef signed int				int32_t;
typedef unsigned int			uint32_t;

typedef uint32_t				register_t;

typedef unsigned int long		size_t;
typedef long int				ssize_t;

typedef long long int			intmax_t;
typedef unsigned long long int	uintmax_t;

typedef long long int			ptrdiff_t;
#  define PTRDIFF_T				unsigned long long int
# elif CONFIG_REGISTER_WIDTH == 64
typedef signed char				int8_t;
typedef unsigned char			uint8_t;

typedef signed short int		int16_t;
typedef unsigned short int		uint16_t;

typedef signed int				int32_t;
typedef unsigned int			uint32_t;

typedef signed long int			int64_t;
typedef unsigned long int		uint64_t;

typedef uint64_t				register_t;

typedef unsigned int long		size_t;
typedef long int				ssize_t;

typedef long long int			intmax_t;
typedef unsigned long long int	uintmax_t;

typedef long long int			ptrdiff_t;
#  define PTRDIFF_T				unsigned long long int
# else // CONFIG_REGISTER_WIDTH
CPP_ASSERT(invalid address width in configuration)
# endif // CONFIG_REGISTER_WIDTH
#else // BUILD_HOST
# include <inttypes.h>
# include <stddef.h>
typedef long int				ssize_t;
# define PTRDIFF_T				unsigned long long int
#endif // BUILD_HOST


#endif // SYS_TYPES_H
