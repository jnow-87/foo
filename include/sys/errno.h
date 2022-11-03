/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_ERRNO_H
#define SYS_ERRNO_H


#include <config/config.h>


/* macros */
#ifdef CONFIG_EXTENDED_ERRNO
# define set_errno(e_code){ \
	errno = e_code; \
	errno_file = __FILE__; \
	errno_line = __LINE__; \
}
#else
# define set_errno(e_code){ \
	errno = e_code; \
}
#endif // CONFIG_EXTENDED_ERRNO

#define reset_errno()				set_errno(0)
#define return_errno(e_code)		{ set_errno(e_code); return (-e_code); }
#define goto_errno(label, e_code)	{ set_errno(e_code); goto label; }


/* types */
typedef enum{
	E_INVAL = 1,	/**< invalid argument, e.g. out of range or entry not found */
	E_NOMEM,		/**< out of memory */
	E_LIMIT,		/**< implementation limit reached */
	E_IO,			/**< I/O error */
	E_NOIMP,		/**< function not implemented */
	E_INUSE,		/**< resource in use */
	E_UNAVAIL,		/**< resource is not available */
	E_AGAIN,		/**< no data available, try again */
	E_END,			/**< end of resource reached */
	E_NOSUP,		/**< operation not supported */
	E_CONN,			/**< connection already established */
	E_NOCONN,		/**< no connection */
	E_UNKNOWN,		/**< unknown error */
} errno_t;


/* external variables */
extern errno_t errno;

#ifdef CONFIG_EXTENDED_ERRNO
extern char const *errno_file;
extern unsigned int errno_line;
#endif // CONFIG_EXTENDED_ERRNO


#endif // SYS_ERRNO_H
