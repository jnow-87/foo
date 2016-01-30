#ifndef SYS_TYPES_H
#define SYS_TYPES_H


#include <config/config.h>


/* macros */
#define false	0
#define true	(!true)


/* architecture independent types */
typedef char	bool;

/* architecture dependent types */
#if CONFIG_ADDR_WIDTH == 8

typedef signed char			int8_t;
typedef unsigned char		uint8_t;

typedef signed short int	int16_t;
typedef unsigned short int	uint16_t;

typedef signed long int		int32_t;
typedef unsigned long int	uint32_t;

typedef void*				addr_t;
typedef uint8_t				register_t;
typedef int					size_t;

#elif CONFIG_ADDR_WIDTH == 32 // CONFIG_ADDR_WIDTH

typedef signed char			int8_t;
typedef unsigned char		uint8_t;

typedef signed short int	int16_t;
typedef unsigned short int	uint16_t;

typedef signed int			int32_t;
typedef unsigned int		uint32_t;

typedef void*				addr_t;
typedef uint32_t			register_t;
typedef int					size_t;

#else // CONFIG_ADDR_WIDTH

#error "invalid address width in configuration"

#endif // CONFIG_ADDR_WIDTH


#endif // SYS_TYPES_H
