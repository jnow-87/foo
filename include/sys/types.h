#ifndef SYS_TYPES_H
#define SYS_TYPES_H


#include <config/config.h>
#include <sys/compiler.h>


/* macros */
#define false	0
#define true	(!false)


/* architecture dependent types */
typedef char					bool;
typedef void					*addr_t;


/* fixed-width types */
#ifndef BUILD_HOST
#if CONFIG_REGISTER_WIDTH == 8

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
#define PTRDIFF_T				unsigned int

#elif CONFIG_REGISTER_WIDTH == 32 // CONFIG_REGISTER_WIDTH

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
#define PTRDIFF_T				unsigned long long int

#else // CONFIG_REGISTER_WIDTH

GCC_ERROR(invalid address width in configuration)

#endif // CONFIG_REGISTER_WIDTH

#else // BUILD_HOST

#include <inttypes.h>
#include <stddef.h>

typedef long int				ssize_t;
#define PTRDIFF_T				unsigned long long int

#endif // BUILD_HOST


#endif // SYS_TYPES_H
