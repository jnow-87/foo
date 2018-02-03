#ifndef SYS_STDARG_H
#define SYS_STDARG_H


#include <config/config.h>


/* macros */
#ifdef CONFIG_AVR

#define va_start(ap, last)	__builtin_va_start(ap[0], last)
#define va_end(ap)			__builtin_va_end(ap[0])
#define va_arg(ap, type)	__builtin_va_arg(ap[0], type)
#define va_copy(dest, src)	__builtin_va_copy(dest[0], src[0])

#else

#define va_start(ap, last)	__builtin_va_start(ap, last)
#define va_end(ap)			__builtin_va_end(ap)
#define va_arg(ap, type)	__builtin_va_arg(ap, type)
#define va_copy(dest, src)	__builtin_va_copy(dest, src)

#endif // CONFIG_AVR

/* types */
#ifdef CONFIG_AVR

// NOTE __builtin_va_list seems to be defined as void pointer. To allow
// 		va_lists being passed between functions with all functions being
// 		affected by updates to the list va_list is defined as an array
// 		type as it is for x86 systems.
typedef __builtin_va_list va_list[1];

#else

typedef __builtin_va_list va_list;

#endif // CONFIG_AVR


#endif // SYS_STDARG_H
